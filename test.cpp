#include "gtest/gtest.h"
#include "CardTools.h"
#include <unordered_set>
#include <concepts>
using card = card_tools::st_card;
using standart_comb = card_tools::pocker_rules<card>::standart_comb;
using comb_name = card_tools::pocker_rules<card>::comb_name;
using comp = card_tools::pocker_rules<card>::PockerCombinationComparator;

template <typename T>
::testing::AssertionResult check_extraction(std::vector<card> table, comb_name res_name, std::array<card, 5> res_comb)
{
    std::sort(table.begin(), table.end());
    auto temp = T::extract_comb(table);
    if (temp.get_name() == res_name && temp.get_comb() == res_comb)
        return ::testing::AssertionSuccess();
    else
        return ::testing::AssertionFailure() << temp;
}

TEST(card_tools::tests::PockerExtractorsTest, CopiesTest) {
    using CareExtractor = card_tools::pocker_rules<card>::CareExtractor;
    std::vector<card> easy_care = { card("KH"), card("KD"), card("KS"), card("KC"), card("AD"), card("AS"), card("4S") };
    std::vector<card> care_concurrent = { card("AH"), card("AD"), card("AS"), card("AC"), card("KD"), card("KS"), card("KH"), card("KC")};

    std::array<card, 5> easy_res = {card("AD"), card("KC"), card("KS"), card("KD"), card("KH")};
    std::array<card, 5> concurrent_res = {card("KH"), card("AC"), card("AS"), card("AD"), card("AH")};

    EXPECT_TRUE(check_extraction<CareExtractor>(easy_care, comb_name::Care, easy_res));
    EXPECT_TRUE(check_extraction<CareExtractor>(care_concurrent, comb_name::Care, concurrent_res));
}

TEST(card_tools::tests::PockerExtractorsTest, TwoPairExtractorTest) {
    using TwoPairExtractor = card_tools::pocker_rules<card>::TwoPairExtractor;
    std::vector<card> easy = { card("KH"), card("KD"), card("9S"), card("5H"), card("7D"), card("5S"), card("3S") };
    std::vector<card> pairs_concurrent = { card("AH"), card("AD"), card("KS"), card("KC"), card("9D"), card("9S"), card("6H"), card("6C") };

    std::array<card, 5> easy_res = {card("9S"), card("5S"), card("5H"), card("KD"), card("KH")};
    std::array<card, 5> concurrent_res = {card("9D"), card("KC"), card("KS"), card("AD"), card("AH")};

    EXPECT_TRUE(check_extraction<TwoPairExtractor>(easy, comb_name::TwoPair, easy_res));
    EXPECT_TRUE(check_extraction<TwoPairExtractor>(pairs_concurrent, comb_name::TwoPair, concurrent_res));
}

TEST(card_tools::tests::PockerExtractorsTest, StraightExtractorTest) {
    using StraightExtractor = card_tools::pocker_rules<card>::StraightExtractor;
    std::vector<card> no_straight = { card("AH"), card("AD"), card("9S"), card("9H"), card("6D"), card("6S"), card("4S") };
    std::vector<card> easy_straight = { card("AH"), card("KD"), card("AS"), card("10H"), card("9D"), card("QS"), card("JS") };
    std::vector<card> ace_straight = { card("AH"), card("KD"), card("JS"), card("5H"), card("4D"), card("3S"), card("2S") };

    std::array<card, 5> easy_res = {card("10H"), card("JS"), card("QS"), card("KD"), card("AH")};
    std::array<card, 5> ace_res = {card("AH"), card("2S"), card("3S"), card("4D"), card("5H")};
    standart_comb to_check_no = StraightExtractor::extract_comb(no_straight);

    EXPECT_EQ(to_check_no, standart_comb());
    EXPECT_TRUE(check_extraction<StraightExtractor>(easy_straight, comb_name::Straight, easy_res));
    EXPECT_TRUE(check_extraction<StraightExtractor>(ace_straight, comb_name::Straight, ace_res));
}

TEST(card_tools::tests::PockerExtractorsTest, FlushExtractorTest) {
    using FlushExtractor = card_tools::pocker_rules<card>::FlushExtractor;
    std::vector<card> comb_h = { card("AS"), card("KS"), card("AH"), card("9S"), card("QS"), card("JS"), card("10S")};
    std::vector<card> comb_concurrent = { card("AS"), card("KS"), card("AH"), card("9S"), card("QS"), card("JS"), card("10S"), card("10H"), card("8H"), card("5H"), card("QH")};

    std::array<card, 5> tr_res_h = { card("10S"), card("JS"), card("QS"), card("KS"), card("AS")};
    std::array<card, 5> tr_res_concurrent ={card("5H"), card("8H"), card("10H"), card("QH"), card("AH")};

    EXPECT_TRUE(check_extraction<FlushExtractor>(comb_h, comb_name::Flush, tr_res_h));
    EXPECT_TRUE(check_extraction<FlushExtractor>(comb_concurrent, comb_name::Flush, tr_res_concurrent));
}

TEST(card_tools::tests::PockerExtractorsTest, FullHouseExtractorTest) {
    using FullHouseExtractor = card_tools::pocker_rules<card>::FullHouseExtractor;
    std::vector<card> no_house = { card("AS"), card("JS"), card("AH"), card("AD"), card("9H"), card("5S"), card("6D") };
    std::vector<card> easy = { card("AS"), card("JS"), card("AH"), card("AD"), card("JH"), card("KS"), card("10S") };
    std::vector<card> low_concurrent = { card("AS"), card("JS"), card("AH"), card("AD"), card("JH"), card("KS"), card("KD") };
    std::vector<card> high_concurrent = { card("QH"), card("QD"), card("AS"), card("AC"), card("5S"), card("QS"), card("AH") };

    std::array<card, 5> easy_res =  { card("JS"), card("JH"), card("AS"), card("AD"), card("AH")};
    std::array<card, 5> low_concurrent_res = { card("KS"), card("KD"), card("AS"), card("AD"), card("AH")};
    std::array<card, 5> high_concurrent_res = { card("QD"), card("QH"), card("AC"), card("AS"), card("AH")};
    standart_comb no_house_check = FullHouseExtractor::extract_comb(no_house);

    EXPECT_EQ(no_house_check, standart_comb());
    EXPECT_TRUE(check_extraction<FullHouseExtractor>(easy, comb_name::FullHouse, easy_res));
    EXPECT_TRUE(check_extraction<FullHouseExtractor>(low_concurrent, comb_name::FullHouse, low_concurrent_res));
    EXPECT_TRUE(check_extraction<FullHouseExtractor>(high_concurrent, comb_name::FullHouse, high_concurrent_res));
}

TEST(card_tools::tests::PockerExtractorsTest, StraightFlushExtractorTest) {
    using StraightFlushExtractor = card_tools::pocker_rules<card>::StraightFlushExtractor;
    std::vector<card> no_str_flush = { card("10S"), card("JS"), card("AH"), card("AD"), card("9H"), card("8S"), card("7S") };
    std::vector<card> lowest_str_flush = { card("AH"), card("2H"), card("3H"), card("AD"), card("4H"), card("8S"), card("5H") };
    std::vector<card> str_flush = { card("10S"), card("JS"), card("AH"), card("AD"), card("9S"), card("8S"), card("7S") };
    std::vector<card> tricky_suits = { card("10S"), card("10H"), card("JH"), card("JC"), card("9H"), card("8H"), card("7H") };
    std::vector<card> royal = { card("AS"), card("JS"), card("QS"), card("KS"), card("JH"), card("9S"), card("10S") };
    std::vector<card> tricky_royal = { card("AS"), card("JS"), card("QS"), card("KS"), card("10C"), card("9C"), card("10S") };

    std::array<card, 5> lowest_str_flush_res = { card("AH"), card("2H"), card("3H"), card("4H"), card("5H")};
    std::array<card, 5> str_flush_res = { card("7S"), card("8S"), card("9S"), card("10S"), card("JS")};
    std::array<card, 5> tricky_suits_res = { card("7H"), card("8H"), card("9H"), card("10H"), card("JH")};
    std::array<card, 5> royal_res = { card("10S"), card("JS"), card("QS"), card("KS"), card("AS")};
    standart_comb no_str_flush_check = StraightFlushExtractor::extract_comb(no_str_flush);

    EXPECT_EQ(no_str_flush_check, standart_comb());
    EXPECT_TRUE(check_extraction<StraightFlushExtractor>(lowest_str_flush, comb_name::StraightFlush, lowest_str_flush_res));
    EXPECT_TRUE(check_extraction<StraightFlushExtractor>(str_flush, comb_name::StraightFlush, str_flush_res));
    EXPECT_TRUE(check_extraction<StraightFlushExtractor>(tricky_suits, comb_name::StraightFlush, tricky_suits_res));
    EXPECT_TRUE(check_extraction<StraightFlushExtractor>(royal, comb_name::Royal, royal_res));
    EXPECT_TRUE(check_extraction<StraightFlushExtractor>(tricky_royal, comb_name::Royal, royal_res));
}

TEST(HoldemTest, DISABLED_ExtractionFulllCheck) {
    std::map<comb_name, unsigned long long int> res;
    std::map<comb_name, unsigned long long int> real_res 
    { {comb_name::Royal, 4324}, {comb_name::StraightFlush, 37260} , {comb_name::Care, 224848} ,
        {comb_name::FullHouse, 3473184 } , {comb_name::Flush, 4047644} , {comb_name::Straight, 6180020} ,
        {comb_name::Set, 6461620} , {comb_name::TwoPair, 31433400} ,{ comb_name::Pair, 58627800 } ,
        {comb_name::HighestCard, 23294460} };
    card_tools::unique_cards_deck<card_tools::st_card> deck;
    size_t general_count = 0;
    for (auto i = deck.get_unordered_sets_generator(7); i.has_next();) {
        auto j = i.get_next();
        std::array<card_tools::st_card, 7> comb;
        std::move(j.begin(), j.begin() + j.size(), comb.begin());
        res[card_tools::holdem_rules::extract_highest_comb(comb).get_name()]++;
        if(++general_count % 10000000 == 0) std::cout << general_count << std::endl;
    }
    std::cout << general_count << "\n";
    EXPECT_EQ(res, real_res);
}