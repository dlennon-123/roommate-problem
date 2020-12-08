#include "include/roommate_split.h"

#include <fstream>
#include <cmath>
#include <stdexcept>
#include <limits>

#include "include/rapidjson/document.h"
#include "include/rapidjson/istreamwrapper.h"

void write_json(Cart &cart, std::map<int, Roommate> &roommates,
                std::string filename)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

    writer.StartObject();
    writer.String("roommates");
    writer.StartArray();
    for (auto &rm : roommates)
    {
        rm.second.json_serialize(writer);
    }
    writer.EndArray();
    writer.String("cart");
    cart.json_serialize(writer);
    writer.EndObject();

    std::ofstream json_output;
    json_output.open(filename);
    json_output << sb.GetString();
    json_output.close();
}

void write_error_json(std::string filename, std::string error_text)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

    writer.StartObject();
    writer.String("error");
    writer.String(error_text.c_str());
    writer.EndObject();

    std::ofstream json_output;
    json_output.open(filename);
    json_output << sb.GetString();
    json_output.close();
}

bool json_has_parent_fields(rapidjson::Document *doc)
{
    if (doc->HasMember("cart") == 0) return false;
    if (doc->HasMember("roommates") == 0) return false;
    return true;
}

bool parse_json_data(Cart *cart, std::map<int, Roommate> *roommates,
                     std::string filename)
{
    bool ret = true;
    std::ifstream ifs(filename);
    rapidjson::IStreamWrapper isw(ifs);

    rapidjson::Document document;
    document.ParseStream(isw);
    if (json_has_parent_fields(&document) == false) return false;

    for (auto &json_rm : document["roommates"].GetArray())
    {
        Roommate *new_rm = new Roommate((*roommates).size(), "");
        new_rm->set_name(json_rm["name"].GetString());
        new_rm->add_to_total(json_rm["total"].GetDouble());
        new_rm->set_tax_share(json_rm["tax_share"].GetDouble());

        for (auto &json_item : json_rm["items"].GetArray())
        {
            new_rm->add_line_item(json_item.GetInt());
        }

        roommates->insert(std::pair<int, Roommate>(new_rm->get_id(), *new_rm));
    }

    (*cart).set_total(document["cart"]["total"].GetDouble());
    (*cart).set_tax(document["cart"]["tax"].GetDouble());
    for (auto &json_li : document["cart"]["line_items"].GetArray())
    {
        Line_item *new_li = new Line_item();
        new_li->set_id(json_li["id"].GetInt());
        new_li->set_name(json_li["item_name"].GetString());
        new_li->set_cost(json_li["cost"].GetDouble());
        new_li->set_share_cost(json_li["share_cost"].GetDouble());
        for (auto &json_int : json_li["splitting"].GetArray())
        {
            new_li->add_splitting(json_int.GetInt());
        }
        cart->add_line_item(*new_li);
    }

    return ret;
}

void calculate_shares(Cart *cart, std::map<int, Roommate> *roommates)
{
    double total_check = 0.0;

    //add items and pre-tax cost to roommates
    for (auto &item : cart->get_line_items())
    {
        const Line_item *lm = &(item.second);
        double share_cost = (*lm).get_cost() / (*lm).get_splitting().size();
        for (int rm_id : lm->get_splitting())
        {
            Roommate *rm = &((*(roommates->find(rm_id))).second);
            rm->add_line_item(*lm);
            rm->add_to_total(share_cost);
            total_check += share_cost;
        }
    }

    double pre_tax_total = cart->get_total() - cart->get_tax();
    for (auto &rm_pair : (*roommates))
    {
        Roommate *rm = &(rm_pair.second);
        double tax_share = (rm->get_total() / pre_tax_total) * cart->get_tax();
        rm->set_tax_share(tax_share);
        rm->add_to_total(tax_share);
        total_check += tax_share;
    }

    if (approximately_equal(total_check, cart->get_total(),
                           std::numeric_limits<double>::epsilon()) == false)
        throw std::logic_error(std::string("Cart total does not match") +
                               std::string("roommate split total"));
}

void validate_input(Cart *cart, std::map<int, Roommate> *roommates)
{
    double temp_total = 0.0;
    if (cart->get_line_items().size() == 0)
        throw std::invalid_argument("Invalid cart items");
    if (cart->get_total() <= 0.0)
        throw std::invalid_argument("Invalid cart total, must be positive");
    if (cart->get_tax() < 0)
        throw std::invalid_argument("Invalid cart tax, cannot be negative");

    for (auto &item : cart->get_line_items())
    {
        const Line_item *lm = &(item.second);
        if (lm->get_cost() < 0)
            throw std::invalid_argument("Item cost is negative id:" +
                                        std::to_string(lm->get_id()));
        temp_total += lm->get_cost();
    }
    temp_total += cart->get_tax();

    if (approximately_equal(temp_total, cart->get_total(),
                           std::numeric_limits<double>::epsilon()) == false)
        throw std::invalid_argument(std::string("Cart cost doesn't equal") +
                                    std::string("sum of all items plus tax"));
}

//Credit to The art of computer programming by Knuth
bool approximately_equal(double a, double b, double epsilon)
{
    return fabs(a - b) <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

//Roommate Implementation
Roommate::Roommate(const int id, std::string name) : id(id), name(name){}
const int Roommate::get_id(){return id;}
std::string Roommate::get_name(){return name;}
void Roommate::set_name(std::string new_name){name = new_name;}
void Roommate::add_line_item(const Line_item &new_item){items.insert(new_item.get_id());}
void Roommate::add_line_item(int item_id){items.insert(item_id);}
void Roommate::add_line_item(std::set<int> item_ids){
    for (int item_id : item_ids) items.insert(item_id);
}
void Roommate::remove_line_item(Line_item &new_item){items.erase(new_item.get_id());}
void Roommate::remove_line_item(int item_id){items.erase(item_id);}
const std::set<int> &Roommate::get_items(){return items;}
void Roommate::add_to_total(double val){total += val;}
double Roommate::get_total(){return total;}
void Roommate::set_tax_share(double val){tax_share = val;}
double Roommate::get_tax_share() {return tax_share;}

std::ostream& operator << (std::ostream &out, const Roommate &r)
{
    out << "id: " << r.id << std::endl << "name: " << r.name << std::endl;
    out << "items: " << std::endl;

    for (auto iter = r.items.begin(); iter != r.items.end(); iter++)
    {
        if (iter != r.items.begin()) out << std::endl;
        out << "id: " << *iter;
    }
    out << std::endl;

    out << "total: " << r.total << std::endl;
    out << "tax_share: " << r.tax_share << std::endl;
    return out;
}

//Line_item Implementation
Line_item::Line_item(int id, std::string name, double item_cost,
          double share_cost, std::set<int> splitting) : id(id),
          name(name), cost(item_cost),
          share_cost(share_cost), splitting(splitting) {}
Line_item::Line_item(){}
void Line_item::set_id(int new_id){id = new_id;}
int Line_item::get_id() const{return id;}
int Line_item::get_id(){return id;}
void Line_item::set_name(std::string new_name){name = new_name;}
std::string Line_item::get_name() const{return name;}
void Line_item::set_cost(double new_cost){cost = new_cost;}
double Line_item::get_cost() const{return cost;}
void Line_item::set_share_cost(double new_cost){share_cost = new_cost;}
double Line_item::get_share_cost() const{return share_cost;}
void Line_item::add_splitting(Roommate &new_rm){
    splitting.insert(new_rm.get_id());
}
void Line_item::add_splitting(int rm_id){splitting.insert(rm_id);}
void Line_item::add_splitting(std::set<int> rm_ids){
    for (int rm_id : rm_ids) splitting.insert(rm_id);
}
const std::set<int> &Line_item::get_splitting() const{return splitting;}

std::ostream& operator << (std::ostream &out,const Line_item &l)
{
    out << "id: " << l.get_id() << " name: " << l.get_name();
    out << " cost: " << l.get_cost() << " shared cost: " << l.get_share_cost();
    out << " rms_split: {";

    for (auto iter = l.get_splitting().begin(); iter != l.get_splitting().end(); iter++)
    {
        if (iter != l.get_splitting().begin()) out << ", ";
        out << *iter;
    }
    out << "}";

    return out;
};

//Cart Implementation
Cart::Cart(){total = 0.0; tax = 0.0;}
Cart::Cart(double total, double tax, const std::map<int, Line_item> items):
     total(total), tax(tax), items(items) {}
void Cart::set_total(double new_val){total = new_val;}
double Cart::get_total() const{return total;}
void Cart::set_tax(double new_val){tax = new_val;}
double Cart::get_tax() const{return tax;}
void Cart::add_line_item(Line_item &new_item){
    items.insert(std::pair<int, Line_item>(new_item.get_id(), new_item));
}
void Cart::remove_line_item(Line_item &new_item){items.erase(new_item.get_id());}
const std::map<int, Line_item> &Cart::get_line_items()const {return items;}

std::ostream& operator << (std::ostream &out, const Cart &c)
{
    out << "total: " << c.get_total() << std::endl;
    out << "tax: " << c.get_tax() << std::endl << "items: " << std::endl;

    for (auto iter = c.get_line_items().begin(); iter != c.get_line_items().end(); iter++)
    {
        if (iter != c.get_line_items().begin()) out << std::endl;
        out << iter->second;
    }

    return out;
}
