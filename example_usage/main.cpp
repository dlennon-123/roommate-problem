#include <iostream>
#include "../include/roommate_split.h"

static std::string json_input = "small_distributed_input.json";
static std::string json_output = "test_json_output.json";
static std::string error_json_output = "Tests/error_json_output.json";

/**
 * Reads input json from <json_input> above, performs share calculation, and
 * outputs the results to <json_output>
 */
int main()
{
    std::map<int, Roommate> roommates = {};
    Cart cart = Cart();
    parse_json_data(&cart, &roommates, std::string(TEST_FILE_PREFIX)
                                       + json_input);

    try
    {
        validate_input(&cart, &roommates);
    }
    catch (std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
        write_error_json(error_json_output, e.what());
        return EXIT_FAILURE;
    }

    try
    {
        calculate_shares(&cart, &roommates);
    }
    catch (std::logic_error& e)
    {
        std::cout << e.what() << std::endl;
        write_error_json(error_json_output, e.what());
        return EXIT_FAILURE;
    }

    //Print all roommates and cart
    for (auto &rm : roommates){std::cout << (rm.second) << std::endl;}
    std::cout << cart;

    write_json(cart, roommates, std::string(EXP_FILE_PREFIX) + json_output);
    return 0;
}
