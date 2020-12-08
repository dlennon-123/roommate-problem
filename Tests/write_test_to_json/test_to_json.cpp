#include <iostream>
#include "../../include/roommate_split.h"

static std::string test_name = "small_distributed";

//Fill in input data for roommates and cart
void make_input_data(Cart *cart, std::map<int, Roommate> *roommates)
{
    Roommate *rm_0 = new Roommate(0, "David");
    Roommate *rm_1 = new Roommate(1, "Erica");
    Roommate *rm_2 = new Roommate(2, "Sarah");

    roommates->insert(std::pair<int, Roommate>(rm_0->get_id(), *rm_0));
    roommates->insert(std::pair<int, Roommate>(rm_1->get_id(), *rm_1));
    roommates->insert(std::pair<int, Roommate>(rm_2->get_id(), *rm_2));

    Line_item *li_0 = new Line_item(0, "chicken breast", 5.67, 0, {0,1,2});
    Line_item *li_1 = new Line_item(1, "milk", 3.25, 0, {0,1});
    Line_item *li_2 = new Line_item(2, "bacon", 2.56, 0, {0});
    Line_item *li_3 = new Line_item(3, "peanut butter", 3.98, 0, {1});

    std::map<int, Line_item> *temp_map = new std::map<int, Line_item>();
    temp_map->insert(std::pair<int, Line_item>(li_0->id, *li_0));
    temp_map->insert(std::pair<int, Line_item>(li_1->id, *li_1));
    temp_map->insert(std::pair<int, Line_item>(li_2->id, *li_2));
    temp_map->insert(std::pair<int, Line_item>(li_3->id, *li_3));

    cart->total = 16.71;
    cart->tax = 1.25;
    cart->items = *temp_map;
}

//Fill in output data for roommates and cart
void make_output_data(Cart *cart, std::map<int, Roommate> *roommates)
{
    roommates->find(0)->second.add_to_total(6.57); //rm_0
    roommates->find(1)->second.add_to_total(8.10); //rm_1
    roommates->find(2)->second.add_to_total(2.04); //rm_2

    roommates->find(0)->second.set_tax_share(0.49); //rm_0
    roommates->find(1)->second.set_tax_share(0.61); //rm_1
    roommates->find(2)->second.set_tax_share(0.15); //rm_2

    roommates->find(0)->second.add_line_item(std::set<int>{0,1,2}); //rm_0
    roommates->find(1)->second.add_line_item(std::set<int>{0,1,3}); //rm_1
    roommates->find(2)->second.add_line_item(std::set<int>{0}); //rm_2
}

//Uses input data in functions above to produce input and expected test files
int main()
{
    std::map<int, Roommate> roommates = {};
    Cart cart = Cart();
    make_input_data(&cart, &roommates);

    try
    {
        validate_input(&cart, &roommates);
    }
    catch (std::invalid_argument& e)
    {
        //json output the error code
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    write_json(cart, roommates, test_name + std::string(TEST_FILE_POSTFIX));

    make_output_data(&cart, &roommates);
    write_json(cart, roommates, test_name + std::string(EXP_FILE_POSTFIX));

    //Print all roommates and cart
    for (auto &rm : roommates){std::cout << (rm.second) << std::endl;}
    std::cout << cart;

    return 0;
}
