#include "CardTools.h"

using namespace card_tools;
st_card::st_card() = default;
st_card::st_card(unsigned int high, unsigned int suit) : generalized_card(high, suit) {}
st_card::st_card(std::string inp) {
    std::smatch m;
    if (!std::regex_match(inp, m, std::regex("((10)|[2-9]|[JQKA])[CSDH]"))) {
        throw std::invalid_argument("Invalid card passed");
    }
    std::regex_search(inp, m, std::regex("(10)|[2-9]|[JQKA]"));
    std::string temp = m[0];
    num = ((temp == "A") ? 12 : ((temp == "K") ? 11 : ((temp == "Q") ? 10 : ((temp == "J") ? 9 : std::stoi(temp) - 2)))) * suits_num;
    std::regex_search(inp, m, std::regex("[CSDH]"));
    temp = m[0];
    num += (temp == "C") ? 0 : ((temp == "S") ? 1 : ((temp == "D") ? 2 : ((temp == "H") ? 3 : -1)));
}

std::string card_tools::to_string(st_card const& self) {
    if (self.get_suit() >= 0 && self.get_high() >= 0) {
        return std::string(st_card::high_names[self.get_high()]) + "" + st_card::suit_names[self.get_suit()];
    }
    return to_string(static_cast<generalized_card<4, 13>>(self));
}

std::ostream& card_tools::operator<<(std::ostream& os, const st_card& m) {
    using std::to_string;
    return os << to_string(m);
}

holdem_rules::holdem_combination::holdem_combination(rules::standart_comb inp) : standart_comb(inp) {}
holdem_rules::holdem_combination::holdem_combination(const river& inp) : holdem_combination(extract_highest_comb(std::vector<st_card>(inp.begin(), inp.begin() + sizeof(inp) / sizeof(inp[0])))) {}

holdem_rules::holdem_combination holdem_rules::extract_highest_comb(const std::vector<st_card>& inp) {
    return rules::extract_comb_ordered<
        rules::StraightFlushExtractor,
        rules::CareExtractor,
        rules::FullHouseExtractor,
        rules::FlushExtractor,
        rules::StraightExtractor,
        rules::SetExtractor,
        rules::TwoPairExtractor,
        rules::PairExtractor,
        rules::HighestCardExtractor
    >(inp);
}

const unsigned int holdem_rules::max_cards_on_desk = 7;
holdem_rules::holdem_combination holdem_rules::extract_highest_comb(const hand& h, const river& r) {
    std::vector<st_card> cards(7);
    std::copy(h.begin(), h.end(), cards.begin());
    std::copy(r.begin(), r.end(), cards.begin() + h.size());
    std::sort(cards.begin(), cards.end());
    return extract_highest_comb(cards);
}

holdem_rules::holdem_combination holdem_rules::extract_highest_comb(const table& t)
{
    std::vector<st_card> cards(t.begin(), t.end());
    return extract_highest_comb(cards);
}

std::map<holdem_rules::comb_name, double> card_tools::holdem_rules::get_all_combs_probability(const hand& h, const river& r, const unique_cards_deck<st_card>& deck)
{
    std::vector<st_card> in_game(7);
    std::copy(h.begin(), h.end(), in_game.begin());
    std::copy(r.begin(), r.end(), in_game.begin() + h.size());
    return get_all_combs_probability(in_game, deck);
}

std::map<holdem_rules::rules::comb_name, double> holdem_rules::get_all_combs_probability(const hand& h, const turn& t, const unique_cards_deck<st_card>& deck)
{
    std::vector<st_card> in_game(6);
    std::copy(h.begin(), h.end(), in_game.begin());
    std::copy(t.begin(), t.end(), in_game.begin() + h.size());
    return get_all_combs_probability(in_game, deck);
}

std::map<holdem_rules::rules::comb_name, double> holdem_rules::get_all_combs_probability(const hand& h, const flop& f, const unique_cards_deck<st_card>& deck)
{
    std::vector<st_card> in_game(5);
    std::copy(h.begin(), h.end(), in_game.begin());
    std::copy(f.begin(), f.end(), in_game.begin() + h.size());
    return get_all_combs_probability(in_game, deck);
}

std::map<holdem_rules::rules::comb_name, double> holdem_rules::get_all_combs_probability(const hand& h, const unique_cards_deck<st_card>& deck)
{
    std::vector<st_card> in_game(3);
    std::copy(h.begin(), h.end(), in_game.begin());
    return get_all_combs_probability(in_game, deck);
}

std::vector<double> card_tools::holdem_rules::get_equity(const unique_cards_deck<st_card>& deck, const river& r, const std::vector<hand>& h)
{
    return get_equity<5>(deck, r, h);
}

std::vector<double> card_tools::holdem_rules::get_equity(const unique_cards_deck<st_card>& deck, const turn& t, const std::vector<hand>& h)
{
    return get_equity<4>(deck, t, h);
}

std::vector<double> card_tools::holdem_rules::get_equity(const unique_cards_deck<st_card>& deck, const flop& f, const std::vector<hand>& h)
{
    return get_equity<3>(deck, f, h);
}

std::vector<double> card_tools::holdem_rules::get_equity(const unique_cards_deck<st_card>& deck, const std::vector<hand>& h)
{
    return get_equity<1>(deck, std::array<st_card, 1>{st_card(-1, -1)}, h);
}

template <unsigned int size>
std::vector<double> card_tools::holdem_rules::get_equity(const unique_cards_deck<st_card>& deck, const std::array<st_card, size>& in_game, const std::vector<hand>& h)
{
    std::vector<double> res(h.size() + 1, 0);
    std::vector<st_card> temp(in_game.begin(), in_game.end());
    if (temp.size() == 1) temp.pop_back();
    size_t general_count = 0;
    for (auto i = deck.get_unordered_sets_generator(max_cards_on_desk - temp.size() - 2); i.has_next();) {
        auto desk = i.get_next();
        standart_comb max;
        size_t winned_player = 0;
        for (size_t j = 0; j < h.size(); j++) {
            std::vector<st_card> iter(temp);
            iter.push_back(st_card());
            iter.push_back(st_card());
            iter[iter.size() - 1] = h[j][0];
            iter[iter.size() - 2] = h[j][1];
            iter.insert(iter.end(), desk.begin(), desk.end());
            std::sort(iter.begin(), iter.end());
            standart_comb temp = extract_highest_comb(iter);
            if (PockerCombinationComparator().compare(max, temp)) {
                winned_player = j;
                max = temp;
            }
        }
        ++general_count;
        ++res[winned_player];
    }
    for (size_t i = 0; i < res.size(); i++) res[i] /= general_count;
    return res;
}

std::map<holdem_rules::rules::comb_name, double> holdem_rules::get_all_combs_probability(std::vector<st_card>& in_game, const unique_cards_deck<st_card>& deck)
{
    std::map<rules::comb_name, double> res;
    std::sort(in_game.begin(), in_game.end());
    size_t general_count = 0;
    for (auto i = deck.get_unordered_sets_generator(max_cards_on_desk - in_game.size()); i.has_next();) {
        std::vector<st_card> temp = i.get_next();
        temp.insert(temp.end(), in_game.begin(), in_game.end());
        std::sort(temp.begin(), temp.end());
        res[extract_highest_comb(i.get_next()).get_name()]++;
        general_count++;
    }
    for (auto i : res) {
        i.second /= general_count;
    }
    return res;
}