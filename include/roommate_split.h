#ifndef ROOMMATE_SPLIT_H_INCLUDED
#define ROOMMATE_SPLIT_H_INCLUDED

#include <iostream>
#include <map>
#include <set>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#define TEST_FILE_PREFIX "Tests/Input/"
#define TEST_FILE_POSTFIX "_input.json"
#define EXP_FILE_PREFIX "Tests/Expected/"
#define EXP_FILE_POSTFIX "_output.json"

//Forward declaration of Line_item required for Roommate declaration
class Line_item;

//Roommate declaration, implementation in roommate_split.cpp
class Roommate
{
    public:
        Roommate(const int id, std::string name);
        const int get_id();
        std::string get_name();
        void set_name(std::string new_name);
        void add_line_item(const Line_item &new_item);
        void add_line_item(int item_id);
        void add_line_item(std::set<int> item_ids);
        void remove_line_item(Line_item &new_item);
        void remove_line_item(int item_id);
        const std::set<int> &get_items();
        void add_to_total(double val);
        double get_total();
        void set_tax_share(double val);
        double get_tax_share();
        friend std::ostream & operator << (std::ostream &out, const Roommate &r);

        template <typename Writer>
        void json_serialize(Writer& writer) const {
            writer.StartObject();
            writer.String("id");
            writer.Int(id);
            writer.String("name");
            writer.String(name.c_str());
            writer.String("items");
            writer.StartArray();
            for (auto &item : items)
            {
                writer.Int(item);
            }
            writer.EndArray();
            writer.String("total");
            writer.Double(total);
            writer.String("tax_share");
            writer.Double(tax_share);
            writer.EndObject();
        }

    private:
        const int id;
        std::string name;
        std::set<int> items = {};
        double total = 0.0;
        double tax_share = 0.0;

};

//Line_item declaration, implementation in roommate_split.cpp
class Line_item
{
    public:
        Line_item(int id, std::string name, double item_cost,
                  double share_cost, std::set<int> splitting);
        Line_item();

        void set_id(int new_id);
        int get_id() const;
        int get_id();
        void set_name(std::string new_name);
        std::string get_name() const;
        void set_cost(double new_cost);
        double get_cost() const;
        void set_share_cost(double new_cost);
        double get_share_cost() const;
        void add_splitting(Roommate &new_rm);
        void add_splitting(int rm_id);
        void add_splitting(std::set<int> rm_ids);
        const std::set<int> &get_splitting() const;

        template <typename Writer>
        void json_serialize(Writer& writer) const {
            writer.StartObject();
            writer.String("id");
            writer.Int(id);
            writer.String("item_name");
            writer.String(name.c_str());
            writer.String("cost");
            writer.Double(cost);
            writer.String("share_cost");
            writer.Double(share_cost);
            writer.String("splitting");
            writer.StartArray();
            for (auto &rm : splitting)
            {
                writer.Int(rm);
            }
            writer.EndArray();
            writer.EndObject();
        }

    private:
        int id;
        std::string name;
        double cost;
        double share_cost;
        std::set<int> splitting = {};

};

//Cart declaration, implementation in roommate_split.cpp
class Cart
{
    public:
        Cart(double total, double tax, const std::map<int, Line_item> items);
        Cart();
        void set_total(double new_val);
        double get_total() const;
        void set_tax(double new_val);
        double get_tax() const;
        void add_line_item(Line_item &new_item);
        void remove_line_item(Line_item &new_item);
        const std::map<int, Line_item> &get_line_items() const;
        friend std::ostream & operator << (std::ostream &out, const Cart &c);

        template <typename Writer>
        void json_serialize(Writer& writer) const {
            writer.StartObject();
            writer.String("total");
            writer.Double(total);
            writer.String("tax");
            writer.Double(tax);
            writer.String("line_items");
            writer.StartArray();
            for (auto &rm : items)
            {
                rm.second.json_serialize(writer);
            }
            writer.EndArray();
            writer.EndObject();
        }

    private:
        double total;
        double tax;
        std::map<int, Line_item> items = {};

};

//Roommate Split functions
void validate_input(Cart *cart, std::map<int, Roommate> *roommates);
void write_json(Cart &cart, std::map<int, Roommate> &roommates,
                std::string filename);
void write_error_json(std::string filename, std::string error_text);
bool json_has_parent_fields(rapidjson::Document *doc);
bool parse_json_data(Cart *cart, std::map<int, Roommate> *roommates,
                     std::string filename);
void calculate_shares(Cart *cart, std::map<int, Roommate> *roommates);
bool approximately_equal(double a, double b, double epsilon);
std::ostream& operator << (std::ostream &out, const Roommate &r);
std::ostream& operator << (std::ostream &out, const Cart &c);
std::ostream& operator << (std::ostream &out, const Line_item &l);

#endif // ROOMMATE_SPLIT_H_INCLUDED
