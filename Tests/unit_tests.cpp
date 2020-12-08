#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE BasicUnitTests
#include <boost/test/unit_test.hpp>

#include "../include/roommate_split.h"
#include <map>
#include <iostream>

/** The tolerance must be 0.02 to account for error in the relative distance calculation.
 *  This limits the tested accuracy to $0.02 for customer totals.
 */
#define ACCEPTABLE_TEST_TOLERANCE 0.02

void run_test(std::string test_name);

BOOST_AUTO_TEST_CASE(all_items_one_roommate)
{
    run_test("all_items_one_roommate");
}

BOOST_AUTO_TEST_CASE(no_items_one_roommate)
{
    run_test("no_items_one_roommate");
}

BOOST_AUTO_TEST_CASE(regular_distributed)
{
    run_test("regular_distributed");
}

BOOST_AUTO_TEST_CASE(larger_distributed)
{
    run_test("larger_distributed");
}

BOOST_AUTO_TEST_CASE(small_distributed)
{
    run_test("small_distributed");
}

/**
 * Reads input json from <test_name>_input.json, performs share calculation,
 * and compares result to <test_name>_output.json
 */
void run_test(std::string test_name)
{
    std::map<int, Roommate> roommates = {};
    Cart cart = Cart();

    std::map<int, Roommate> exp_roommates = {};
    Cart exp_cart = Cart();

    parse_json_data(&cart, &roommates,
                    (std::string(TEST_FILE_PREFIX) +
                     test_name +
                     std::string(TEST_FILE_POSTFIX)));

    try{ validate_input(&cart, &roommates); }
    catch (std::invalid_argument& e)
    {
        //json output the error code
        std::cout << e.what() << std::endl;
        exit(1);
    }

    try{ calculate_shares(&cart, &roommates);}
    catch (std::logic_error& e)
    {
        //json output the error code
        std::cout << e.what() << std::endl;
        exit(1);
    }

    //The expected output JSON file is gaurenteed to be valid
    parse_json_data(&exp_cart, &exp_roommates,
                    (std::string(EXP_FILE_PREFIX) +
                     test_name +
                     std::string(EXP_FILE_POSTFIX)));

    BOOST_TEST(roommates.size() == exp_roommates.size());
    for (auto &rm : roommates)
    {
        Roommate *first = &(rm.second);
        Roommate *second = &((exp_roommates.find(first->get_id()))->second);
        BOOST_TEST(first->get_name().compare(second->get_name()) == 0);
        BOOST_TEST(first->get_total() == second->get_total(),
                   boost::test_tools::tolerance(ACCEPTABLE_TEST_TOLERANCE));
        BOOST_TEST(first->get_tax_share() == second->get_tax_share(),
                   boost::test_tools::tolerance(ACCEPTABLE_TEST_TOLERANCE));
        BOOST_TEST(first->get_items().size() == second->get_items().size());
        const std::set<int> &second_items = second->get_items();
        for (auto &item : first->get_items())
        {
            BOOST_TEST(second_items.count(item) == 1);
        }
    }

}
