#ifndef L_SYSTEM_H
#define L_SYSTEM_H


#include <string>
#include <unordered_map>
#include <iostream>
#include <algorithm>

#include "cereal/cereal.hpp"
#include "cereal/types/unordered_map.hpp"

#include "Observable.h"
#include "RuleMap.h"


namespace cereal
{
    // Saving for std::map<std::string, std::string> for text based archives
    // Copied from https://uscilab.github.io/cereal/archive_specialization.html (MIT License)
    template <class Archive, class C, class A,
              traits::EnableIf<traits::is_text_archive<Archive>::value> = traits::sfinae> inline
    void save( Archive & ar, std::unordered_map<char, std::string, C, A> const& map )
    {
        for(const auto& i : map)
            ar(cereal::make_nvp(std::string()+i.first, i.second));
    }

    // Loading for std::map<char, std::string> for text based archives
    template <class Archive, class C, class A,
              traits::EnableIf<traits::is_text_archive<Archive>::value> = traits::sfinae> inline
    void load( Archive & ar, std::unordered_map<char, std::string, C, A>& map)
    {
        map.clear();

        auto hint = map.begin();
        while(true)
        {
            const auto namePtr = ar.getNodeName();

            if(!namePtr)
                break;

            std::string key = namePtr;
            std::string value; ar(value);
            hint = map.emplace_hint(hint, key.at(0), std::move (value));
        }
    }
}

// Simple L-system generation class. Starting from an axiom and simple
// production rules, generate by iteration a result array of character by
// replacing each character predecessor by its word successor. It also produces
// an array containing for each character the derivation count corresponding to
// the number of iteration of this character was derived from a particular rule.
//
// For example:
//   axiom: "F"
//   production rules:
//     "F" -> "F+G" (<- predecessor to save iteration count)
//     "G" -> "F-G"
//   iteration 0:
//     production_array: "F"
//     iteration_array:  {0}
//   iteration 1:
//     production_array: "F + G" (without space)
//     iteration_array:  {1,1,1}
//   iteration 2:
//     production_array: "F + G  +  F - G" (without space)
//     iteration_array:  {2,2,2, 1, 1,1,1}
//
// This class is a simple variant of more general
// L-systems: context-free (one generating symbol by rule) and
// deterministic (at most one rule for each symbol).
// Invariants:
//   - If an axiom is defined at construction, 'production_cache_.at(0)'
//   contains it at all time. And 'iteration_count_cache_.at(0)' is 0.
//   - 'iteration_count_cache_' is coherent with
//  'iteraction_count_predecessors_' 
//   - 'production_cache_' and 'iteration_count_cache_' are coherent with the
//  'rules_'. 
//   - As a consequence, 'iteration_count_cache_' and 'production_cache_' are
//   coherent BUT 'iteration_count_cache_' may have less elements than
//   'production_cache_'. In any case, 'iteration_count_cache_' has always less
//   or equal element than'production_cache_'.
class LSystem : public RuleMap<std::string>
{
public:
    // A 'production_rule' is a sole symbol associated with an
    // array of symbols. In an iteration, each symbol will be
    // replaced by its associated array. Symbols without a rule
    // (terminals) are replaced by themselves.  The rules are
    // contained in a hashmap for quick access during an
    // iteration.
    using production_rules = RuleMap::rule_map;
        
    // Constructors
    LSystem() = default;
    LSystem(const std::string& axiom, const production_rules& prod, const std::string& preds);

    // Getters and setters
    std::string get_axiom() const;
    const std::unordered_map<int, std::string>& get_production_cache() const;
    std::string get_iteration_predecessors() const;
    const std::unordered_map<int, std::pair<std::vector<int>, int>>& get_iteration_cache() const;

    // Set the axiom to 'axiom'
    void set_axiom(const std::string& axiom);

    // Add the rule "predecessor -> successor"
    // Note: replace the successor of an existing rule if 'predecessor' has
    // already a rule associated.
    void add_rule(char predecessor, const RuleMap::successor& successor) override;

    // Remove the rule associated to 'predecessor'
    // Exception:
    //   - Precondition: 'predecessor' must have a rule associated.
    void remove_rule(char predecessor) override;

    // Clear the rules
    void clear_rules() override;

    // Set the symbols flagging the iteration count 'iteration_predecessors_' to
    // 'predecessors'.
    void set_iteration_predecessors(const std::string& predecessors);

    // Returns from the first part the result of the 'n'-th iteration of the
    // L-System and cache it as well as the transitional iterations. For the
    // second part, returns the array indicating the derivation number of rules
    // having for predecessors any character from 'iteration_predecessors_'. For
    // the third part, return the maximum number of iteration.
    //
    // Exceptions:
    //   - Precondition: n positive.
    //   - Ensures coherence of 'production_rules
    //   - Throw in case of allocation problem.
    //   - Throw at '.at()' if code is badly refactored.
    std::tuple<std::string, std::vector<int>, int> produce(int n);
       
private:

    friend class cereal::access;
    
    template <class Archive>
    void save (Archive& ar, const std::uint32_t) const
        {
            ar(cereal::make_nvp("axiom", production_cache_.at(0)),
               cereal::make_nvp("production_rules", rules_),
               cereal::make_nvp("iteration_predecessor", iteration_predecessors_));
        }
    
    template <class Archive>
    void load (Archive& ar, const std::uint32_t)
        {
            ar(cereal::make_nvp("axiom", production_cache_[0]),
               cereal::make_nvp("production_rules", rules_),
               cereal::make_nvp("iteration_predecessor", iteration_predecessors_));
            iteration_count_cache_[0] = {std::vector<int>(production_cache_.at(0).size(), 0), 0};
        }

    // The predecessors indicating than, at their next derivation, the iteration
    // counter will be incremented by one.
    std::string iteration_predecessors_ = {};

    // The cache of all computed iterations and the axiom.
    // It contains all the iterations up to the highest iteration
    // calculated. It is clearly not optimized for memory
    // usage. However, this project emphasizes interactivity so
    // quickly swapping between different iterations of the same
    // L-System.
    std::unordered_map<int, std::string> production_cache_ = {};
    // The cache of all computed iteration values. The second element in the pair
    // is the maximum number of iteration for this iteration.
    std::unordered_map<int, std::pair<std::vector<int>, int>> iteration_count_cache_ = {};
};

#endif

