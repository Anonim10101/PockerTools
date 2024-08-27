#pragma once
#include <iostream>
#include <unordered_set>
#include <vector>
#include <math.h>
#include <algorithm>
#include <array>
#include <map>
#include <mutex>
#include <optional>
#include <list>
#include <type_traits>
#include <string>
#include <stdexcept>
#include <regex>
#include <concepts>
#include <random>
#include "gtest/gtest.h"

namespace card_tools {
    namespace tests {
        class PockerExtractorsTest;
        class HoldemExtractionTest;
        class PockerExtractorsTest_CopiesTest_Test;
        class PockerExtractorsTest_TwoPairExtractorTest_Test;
        class PockerExtractorsTest_StraightExtractorTest_Test;
        class PockerExtractorsTest_FlushExtractorTest_Test;
        class PockerExtractorsTest_FullHouseExtractorTest_Test;
        class PockerExtractorsTest_StraightFlushExtractorTest_Test;
    }

    namespace tools {
        template<template<unsigned int, unsigned int> class T, class U>
        struct isDerivedFrom
        {
        private:
            template<unsigned int a, unsigned int  b>
            static decltype(static_cast<const T<a, b>&>(std::declval<U>()), std::true_type{})
                test(const T<a, b>&);

            static std::false_type test(...);
        public:
            static constexpr bool value = decltype(isDerivedFrom::test(std::declval<U>()))::value;
        };

        struct memorizing_C {
        public:
            double C(uint64_t name, uint64_t k) {
                if (k > name) {
                    return 0;
                }
                if (k == 0 || k == name)
                    return 1;
                if (calculated.find({ name, k }) == calculated.end()) {
                    calculated[{name, k}] = C(name - 1, k - 1) * name / k;
                }
                return calculated[{name, k}]; ;
            }
        private:
            std::map<std::pair<uint64_t, uint64_t>, double> calculated;
        };
    }
   

    template <unsigned int s_n, unsigned int hg>
    struct generalized_card {
    public:
        static constexpr unsigned int suits_num = s_n;
        static constexpr unsigned int hg_num = hg;

        generalized_card() = default;
        generalized_card(unsigned int high, unsigned int suit) : num(suits_num * high + suit) {}

        friend std::string to_string(generalized_card<s_n, hg> const& self) {
            return std::to_string(self.get_high()) + "_" + std::to_string(self.get_suit());
        }
        friend std::ostream& operator<<(std::ostream& os, const generalized_card<s_n, hg>& m) {
            using std::to_string;
            return os << to_string(m);
        }
        template<unsigned int s_n, unsigned int hg>
        friend bool operator<(const generalized_card<s_n, hg>& a, const generalized_card<s_n, hg>& b) {
            return a.get_high() < b.get_high() || (a.get_high() == b.get_high() && a.get_suit() < b.get_suit());
        }
        template<unsigned int s_n, unsigned int hg>
        friend bool operator>=(const generalized_card<s_n, hg>& a, const generalized_card<s_n, hg>& b) {
            return !(a < b);
        }
        template<unsigned int s_n, unsigned int hg>
        friend bool operator>(const generalized_card<s_n, hg>& a, const generalized_card<s_n, hg>& b) {
            return b < a;
        }
        template<unsigned int s_n, unsigned int hg>
        friend bool operator<=(const generalized_card<s_n, hg>& a, const generalized_card<s_n, hg>& b) {
            return !(a > b);
        }
        template<unsigned int s_n, unsigned int hg>
        friend bool operator==(const generalized_card<s_n, hg>& a, const generalized_card<s_n, hg>& b) {
            return (a.get_high() == b.get_high()) && (a.get_suit() == b.get_suit());
        }
        template<unsigned int s_n, unsigned int hg>
        friend bool operator!=(const generalized_card<s_n, hg>& a, const generalized_card<s_n, hg>& b) {
            return !(a == b);
        }
        unsigned int get_suit() const {
            return num % suits_num;
        }
        unsigned int get_high() const {
            return num / suits_num;
        }
        generalized_card& operator++() {
            num = (num + 1) % (suits_num * hg_num);
            return *this;
        }
        generalized_card operator++(int) {
            return (*this).operator++();
        }
        static size_t cards_exists() {
            return hg_num * suits_num;
        }
        friend struct CardHashFunction;
    protected:
        int num = 0;
    };

    struct CardHashFunction
    {
        template <unsigned int s_n, unsigned int hg_num>
        size_t operator()(const generalized_card<s_n, hg_num>& card) const
        {
            return  std::hash<int>()(card.num);
        }
    };

    class st_card : public generalized_card<4, 13> {
    public:
        st_card();
        st_card(std::string inp);
        st_card(unsigned int high, unsigned int suit);
        friend std::string to_string(st_card const& self);
        friend std::ostream& operator<<(std::ostream& os, const st_card& m);
    private:
        constexpr static char suit_names[4] = { 'C', 'S','D', 'H' };
        constexpr static std::array high_names = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A" };
    };
    template <typename Card> requires(tools::isDerivedFrom<generalized_card, Card>::value && std::constructible_from<Card, int, int>)
        class unique_cards_deck {
        public:
            class OutOfDeckException
            {
            public:
                OutOfDeckException(std::string message) : message{ message } {}
                std::string getMessage() const { return message; }
            private:
                std::string message;
            };
            struct state {
                std::unordered_set<Card, CardHashFunction> out;
            };
            struct set_generator {
                bool has_next() {
                    return is_end;
                }
                //  this function does not support multithreading - use get_if_has if needed
                std::vector<Card> get_next() { 
                    std::vector<Card> res(temp_set.size());
                    for (size_t i = 0; i < res.size(); i++) res[i] = in_deck[temp_set[i]];
                    gen_next();
                    return res;
                }
                std::optional<std::vector<Card>> get_if_has() {
                    std::lock_guard<std::mutex>(m);
                    if (has_next()) return get_next();
                    return std::optional<std::vector<Card>>();
                }
            private:
                friend unique_cards_deck;
                set_generator() = default;
                set_generator(const std::unordered_set<Card, CardHashFunction>& out, size_t in_set) : temp_set(std::vector<size_t>(in_set)) {
                    size_t num = 0;
                    while (num < Card::cards_exists()) {
                        Card temp;
                        do {
                            temp = Card(num / Card::suits_num, num % Card::suits_num);
                            ++num;
                        } while (out.contains(temp) && num < Card::cards_exists());
                        if (!out.contains(temp)) in_deck.push_back(temp);
                    }
                    for (size_t i = 0; i < in_set; i++) temp_set[i] = i;
                    is_end = in_set <= in_deck.size();
                }
                void gen_next() {
                    is_end = increment(temp_set.size() - 1);
                }
                bool increment(size_t num) {
                    if (temp_set[num] == in_deck.size() - (temp_set.size() - num)) {
                        if (num == 0 || !increment(num - 1)) {
                            return false;
                        }
                        temp_set[num] = temp_set[num - 1] + 1;
                    }
                    else ++temp_set[num];
                    return true;
                }
                std::mutex m;
                std::vector<Card> in_deck;
                std::vector<size_t> temp_set;
                bool is_end;

            };
            unique_cards_deck() {}
            unique_cards_deck(std::unordered_set<Card, CardHashFunction> out) : temp_state{ out } {}
            const state& get_state() {
                return temp_state;
            }
            unsigned int cards_in_deck() {
                return Card::cards_exists() - temp_state.out.size();
            }
            bool draw_card(Card to_rem) {
                return temp_state.out.insert(to_rem).second;
            }
            bool draw_card(std::vector<Card> to_rem) {
                bool res = true;
                for (auto i : to_rem) {
                    if (!draw_card(i))  throw OutOfDeckException{ "Card" + to_string(i) + " out of deck" };
                }
                return res;
            }
            Card draw_rand() {
                if (temp_state.out.size() >= Card::cards_exists()) {
                     throw OutOfDeckException{"Deck has no cards"};
                }
                size_t dec_num = rng() % (Card::cards_exists() - temp_state.out.size());
                Card temp_card(-1, -1);
                size_t counter = 0, num = 0;
                do {
                    temp_card = Card((num / Card::suits_num) % Card::hg_num, num % Card::suits_num);
                    if (temp_state.out.find(temp_card) == temp_state.out.end()) counter++;
                    num++;
                } while (counter < dec_num);
                temp_state.out.insert(temp_card);
                return temp_card;
            }

            set_generator get_unordered_sets_generator(size_t size) const{
                return set_generator(temp_state.out, size);
            }

            double probability_of_getting(std::vector<Card> inp, unsigned int tries_num) {
                double res = 0;
                for (size_t i = 0; i < inp.size(); i++) {
                    if (temp_state.out.find(inp[i]) != temp_state.out.end()) {
                        return res;
                    }
                }
                double accum = 1;
                for (size_t i = 0; i < tries_num; i++) {
                    double comb_num = num_calc.C((unsigned long long int)(Card::hg_num) * Card::suits_num - i * inp.size(), inp.size());
                    res += accum / comb_num;
                    accum *= num_calc.C((unsigned long long int)(Card::hg_num) * Card::suits_num - (i + 1) * inp.size(), inp.size()) / comb_num;
                }
                return res;
            }
        private:
            std::mt19937 rng = std::mt19937();
            state temp_state;
            tools::memorizing_C num_calc;
    };

    template <typename Card>  requires(tools::isDerivedFrom<generalized_card, Card>::value)
        class pocker_rules {
        public:
            template <unsigned int>
            struct pocker_combination;
            using standart_comb = pocker_combination<5>;

            enum class comb_name {
                Unresolved, HighestCard, Pair, TwoPair, Set, Straight, Flush, FullHouse, Care, StraightFlush, Royal
            };

            template <unsigned int comb_size>
            struct pocker_combination {
                pocker_combination() = default;
                template <typename ...T>
                pocker_combination(std::array<Card, comb_size> inp) {
                    (*this) = pocker_rules::extract_comb_ordered<T...>(inp);
                }

                static constexpr unsigned int get_comb_size() {
                    return comb_size;
                }

                comb_name get_name() const {
                    return name;
                }

                std::array<Card, comb_size> get_comb() const {
                    return comb;
                }
                friend std::string to_string(pocker_combination<comb_size> const& self) {
                    std::string res;
                    using std::to_string;
                    for (auto i: self.get_comb()) res += " " + to_string(i);
                    return to_string((int)self.get_name()) + res;
                }

                friend std::ostream& operator<<(std::ostream& os, const pocker_combination<comb_size>& m) {
                    using std::to_string;
                    return os << to_string(m);
                }

                friend bool operator==(const pocker_combination& a, const pocker_combination& b) {
                    for (size_t i = 0; i < a.comb.size(); i++) {
                        if (a.comb[i] != b.comb[i]) return false;
                    }
                    return a.get_name() == b.get_name();
                }
                friend bool operator!=(const pocker_combination& a, const pocker_combination& b) {
                    return !(a == b);
                }
                protected:
                friend struct BasePockerExtractor;
                pocker_combination(comb_name name, std::array<Card, comb_size> comb) : name{ name }, comb{ comb } {};

                comb_name name = comb_name::Unresolved;
                std::array<Card, comb_size> comb = { Card(0, 0), Card(0, 0) , Card(0, 0) , Card(0, 0) , Card(0, 0) };
            };

            struct BaseCombinationComparator {
                virtual bool compare(const standart_comb& a, const standart_comb& b) = 0;
            };

            struct PockerCombinationComparator : BaseCombinationComparator {
                virtual bool compare(const standart_comb& a, const standart_comb& b) {
                    if (a.get_name() != b.get_name()) return a.get_name() < b.get_name();
                    bool res = false;
                    const size_t last = a.get_comb().size() - 1;
                    for (int i = a.get_comb().size() - 1; i > 0; i--) {
                        if (a.get_comb()[i].get_high() != b.get_comb()[i].get_high()) {
                            res = a.get_comb()[i].get_high() < b.get_comb()[i].get_high();
                            break;
                        }
                    }
                    return res;
                }
                // the cards in the comb go in the order of their comparison in case of equality of combinations, i.e.
                // senior/straight/flush/royal -> sort
                //pair/pairs/three/(three, then a pair)/four , then the remaining ones 
                // when straight with an ace , the ace goes first
            };

            struct BasePockerExtractor {
                // extractors input must be sorted
                static standart_comb extract_comb(const std::vector<Card>& inp) {
                    return standart_comb();
                }
            protected:
                template <typename T, typename B>
                static standart_comb construct_comb(T&& name, B&& comb) {
                    return standart_comb(std::forward<T>(name), std::forward<B>(comb));
                }
                static void set_name(standart_comb& comb, comb_name name) {
                    comb.name = name;
                }
                static auto get_array_begin(standart_comb& comb) {
                    return comb.comb.begin();
                }
                static auto get_array_end(standart_comb& comb) {
                    return comb.comb.end();
                }
                static void fill_with_highest(size_t filled_from, const std::vector<Card>& temp, standart_comb& res) {
                    size_t size = res.comb.size();
                    for (size_t i = temp.size() - 1; filled_from > 0; i--) {
                        if (!is_in_comb<standart_comb::get_comb_size()>(filled_from, res.comb, temp[i])) res.comb[--filled_from] = temp[i];
                    }
                }

                template <unsigned int get_comb_size>
                static bool is_in_comb(size_t filled_from, const std::array<Card, get_comb_size>& comb, const Card& to_check) {
                    for (size_t i = filled_from; i < comb.size(); i++) {
                        if (comb[i] == to_check) return true;
                    }
                    return false;
                }
            };

            struct HighestCardExtractor : BasePockerExtractor {
                static standart_comb extract_comb(const std::vector<Card>& inp) {
                    standart_comb res;
                    BasePockerExtractor::set_name(res, comb_name::HighestCard);
                    BasePockerExtractor::fill_with_highest(standart_comb::get_comb_size(), inp, res);
                    return res;
                }
            };

            template <unsigned int copies_num, comb_name name>
            struct PockerCopiesExtractor : BasePockerExtractor {
                static standart_comb extract_comb(const std::vector<Card>& inp) {
                    standart_comb res;
                    int32_t temp_ind = -1, copies = 1;
                    for (size_t i = 1; i < inp.size(); i++) {
                        if (inp[i - 1].get_high() == inp[i].get_high()) {
                            if (++copies == copies_num) temp_ind = i - copies_num + 1;
                        }
                        else {
                            copies = 1;
                        }
                    }
                    if (temp_ind != -1) {
                        std::copy(inp.begin() + temp_ind, inp.begin() + temp_ind + copies_num, BasePockerExtractor::get_array_end(res) - copies_num);
                        BasePockerExtractor::fill_with_highest(standart_comb::get_comb_size() - copies_num, inp, res);
                        BasePockerExtractor::set_name(res, name);
                    }
                    return res;
                }
                FRIEND_TEST(card_tools::tests::PockerExtractorsTest, CopiesTest);
            };

            using PairExtractor = PockerCopiesExtractor<2, comb_name::Pair>;
            using SetExtractor = PockerCopiesExtractor<3, comb_name::Set>;
            using CareExtractor = PockerCopiesExtractor<4, comb_name::Care>;

            struct TwoPairExtractor : BasePockerExtractor {
                static standart_comb extract_comb(const std::vector<Card>& inp) {
                    standart_comb res;
                    int32_t temp1_ind = -1, temp2_ind = -1;
                    for (size_t i = 1; i < inp.size(); i++) {
                        if (inp[i - 1].get_high() == inp[i].get_high()) {
                            std::swap(temp1_ind, temp2_ind);
                            temp2_ind = i - 1;
                        }
                    }
                    if (temp1_ind != -1) {
                        std::copy(inp.begin() + temp2_ind, inp.begin() + temp2_ind + 2, BasePockerExtractor::get_array_end(res) - 2);
                        std::copy(inp.begin() + temp1_ind, inp.begin() + temp1_ind + 2, BasePockerExtractor::get_array_end(res) - 4);
                        BasePockerExtractor::fill_with_highest(standart_comb::get_comb_size() - 4, inp, res);
                        BasePockerExtractor::set_name(res, comb_name::TwoPair);
                    }
                    return res;
                }
                FRIEND_TEST(card_tools::tests::PockerExtractorsTest, TwoPairExtractorTest);
            };

            struct StraightExtractor : BasePockerExtractor {
                static standart_comb extract_comb(const std::vector<Card>& inp) {
                    size_t ind = 0, counter = 1;
                    for (size_t i = 0; i < inp.size(); i++) {
                        if ((inp[(i + inp.size() - 1) % inp.size()].get_high() + 1) % Card::hg_num == inp[i].get_high()) {
                            if (++counter >= 5) ind = i;
                        }
                        else if (inp[(i + inp.size() - 1) % inp.size()].get_high() != inp[i].get_high()) {
                            counter = 1;
                        }
                        else if (counter >= 5) {
                            ind = i;
                        }
                    }
                    if (ind != 0) {
                        counter = 5;
                        std::array<Card, 5> comb;
                        for (size_t i = ind; counter > 0; i = (i + inp.size()  - 1) % inp.size()) {
                            if (counter == 5 || (comb[counter].get_high() != inp[i].get_high())) comb[--counter] = inp[i];
                        }
                        return BasePockerExtractor::construct_comb(comb_name::Straight, std::move(comb));
                    }
                    return standart_comb();
                }
                FRIEND_TEST(card_tools::tests::PockerExtractorsTest, StraightExtractorTest);
            };

            struct FlushExtractor : BasePockerExtractor {
                static standart_comb extract_comb(const std::vector<Card>& inp) {
                    std::vector<size_t> suits(Card::suits_num, 0);
                    size_t highest_flush = 0;
                    for (size_t i = 0; i < inp.size(); i++) {
                        if ((++suits[inp[i].get_suit()] >= 5) && (highest_flush < inp[i].get_suit())) highest_flush = inp[i].get_suit();
                    }
                    if (suits[highest_flush] >= 5) {
                        std::array<Card, 5> comb;
                        size_t to_fill = 5;
                        for (int i = inp.size() - 1; to_fill > 0; i--) {
                            if (inp[i].get_suit() == highest_flush) comb[--to_fill] = inp[i];
                        }
                        return BasePockerExtractor::construct_comb(comb_name::Flush, std::move(comb));
                    }
                    return standart_comb();
                }
                FRIEND_TEST(card_tools::tests::PockerExtractorsTest, FlushExtractorTest);
            };

            struct FullHouseExtractor : BasePockerExtractor {
                static standart_comb extract_comb(const std::vector<Card>& inp) {
                    int ind1 = -1, ind2 = -2, ind_st = -1;
                    size_t temp_c = 1;
                    for (size_t i = 1; i < inp.size(); i++) {
                        if (inp[i - 1].get_high() == inp[i].get_high()) {
                            if (++temp_c >= 3) {
                                if(temp_c == 3) ind1 = std::max(ind_st, ++ind2);
                                ind2 = i - 2;
                            }
                            else if(temp_c == 2) {
                                ind_st = ind1;
                                ind1 = i - 1;
                            }
                        }
                        else {
                            temp_c = 1;
                        }
                    }
                    if (ind2 >= 0 && ind1 >= 0) {
                        std::array<Card, 5> comb;
                        std::copy(inp.begin() + ind2, inp.begin() + ind2 + 3, comb.end() - 3);
                        std::copy(inp.begin() + ind1, inp.begin() + ind1 + 2, comb.end() - 5);
                        return BasePockerExtractor::construct_comb(comb_name::FullHouse, std::move(comb));
                    }
                    return standart_comb();
                }
                FRIEND_TEST(card_tools::tests::PockerExtractorsTest, FullHouseExtractorTest);
            };

            // suits <= 64, cards in vector <= 9
            struct StraightFlushExtractor : BasePockerExtractor {
                static standart_comb extract_comb(const std::vector<Card>& inp) {
                    std::array<unsigned int, Card::suits_num> suits;
                    for (size_t i = 0; i < suits.size(); i++) suits[i] = 0;
                    size_t max_i = 0, prev = 0, counter = 0;
                    for (size_t i = 0; i < inp.size(); i++) {
                        if (++suits[inp[i].get_suit()] > suits[max_i]) max_i = inp[i].get_suit();
                        if ((inp[i].get_high() == Card::hg_num - 1) && (inp[i].get_suit() == max_i)) {
                            prev = i;
                            counter = 1;
                        }
                    }
                    size_t ind = 0;
                    if (suits[max_i] >= 5) {
                        for (size_t i = 0; i < inp.size(); i++) {
                            if (inp[i].get_suit() == max_i) {
                                if (counter == 0) counter++;
                                else if ((inp[prev].get_high() + 1) % Card::hg_num == inp[i].get_high()) counter++;
                                else counter = 1;
                                prev = i;
                                if (counter >= 5) ind = i;
                            }
                        }
                    }
                    if (ind != 0) {
                        std::array<Card, 5> comb;
                        comb_name name = (inp[ind].get_high() == Card::hg_num - 1)? comb_name::Royal : comb_name::StraightFlush;
                        counter = 5;
                        for (size_t i = ind; counter > 0; i = (i + inp.size() - 1) % inp.size()) {
                            if (counter == 5 || (comb[counter].get_high() != inp[i].get_high() && (inp[i].get_suit() == comb[4].get_suit()))) comb[--counter] = inp[i];
                        }
                        return BasePockerExtractor::construct_comb(name, std::move(comb));
                    }
                    return standart_comb();
                }
            };

            template <typename H, typename S, typename ...T> requires(std::derived_from<H, BasePockerExtractor>)
            static standart_comb extract_comb_ordered(const std::vector<Card>& inp) {
                standart_comb temp_res = H::extract_comb(inp);
                if (temp_res.get_name() != comb_name::Unresolved) return temp_res;
                return extract_comb_ordered<S, T...>(inp);
            }

            template<typename T> requires(std::derived_from<T, BasePockerExtractor>)
            static standart_comb extract_comb_ordered(const std::vector<Card>& inp) {
                return T::extract_comb(inp);
            }

            template <typename Comp, typename H, typename S, typename ...T> requires(std::derived_from<H, BasePockerExtractor> && std::derived_from<Comp, BaseCombinationComparator>)
            static standart_comb get_comb_unordered(Comp& comp, const std::vector<Card>& inp) {
                auto this_res = H::extract_comb(inp);
                auto accum_res = get_comb_unordered<Comp, S, T...>(comp, inp);
                return comp.compare(this_res, accum_res)? accum_res : this_res;
            }
            
            template <typename Comp, typename H> requires(std::derived_from<H, BasePockerExtractor> && std::derived_from<Comp, BaseCombinationComparator>)
            static standart_comb get_comb_unordered(Comp& comp, const std::vector<Card>& inp) {
                return H::extract_comb(inp);
            }

    };

    class holdem_rules : public pocker_rules<st_card> {
    public:
        using table = std::array<st_card, 7>;
        using river = std::array<st_card, 5>;
        using turn = std::array<st_card, 4>;
        using flop = std::array<st_card, 3>;
        using hand = std::array<st_card, 2>;
        using rules = pocker_rules<st_card>;

        struct holdem_combination : pocker_rules::standart_comb {
            holdem_combination(rules::standart_comb inp);
            holdem_combination(const river& inp);
        };

        static holdem_combination extract_highest_comb(const hand& h, const river& r);
        static holdem_combination extract_highest_comb(const table& t);
        static std::map<comb_name, double> get_all_combs_probability(const hand& h, const river& r, const unique_cards_deck<st_card>& deck);
        static std::map<comb_name, double> get_all_combs_probability(const hand& h, const turn& t, const unique_cards_deck<st_card>& deck);
        static std::map<comb_name, double> get_all_combs_probability(const hand& h, const flop& t, const unique_cards_deck<st_card>& deck);
        static std::map<comb_name, double> get_all_combs_probability(const hand& h, const unique_cards_deck<st_card>& deck);
        static std::vector<double> get_equity(const unique_cards_deck<st_card>& deck, const river& r, const std::vector<hand>& h);
        static std::vector<double> get_equity(const unique_cards_deck<st_card>& deck, const turn& t, const std::vector<hand>& h);
        static std::vector<double> get_equity(const unique_cards_deck<st_card>& deck, const flop& f, const std::vector<hand>& h);
        static std::vector<double> get_equity(const unique_cards_deck<st_card>& deck, const std::vector<hand>& h);
    private:
        static const unsigned int max_cards_on_desk;
        template <unsigned int size>
        static std::vector<double> get_equity(const unique_cards_deck<st_card>& deck, const std::array<st_card, size>& in_game, const std::vector<hand>& h);
        static std::map<rules::comb_name, double> get_all_combs_probability(std::vector<st_card>& in_game, const unique_cards_deck<st_card>& deck);
        static holdem_combination extract_highest_comb(const std::vector<st_card>& inp); //input must be sorted
    };
}
