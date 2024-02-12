#include "Parser.h"


int main()
{
    setlocale(0, ".1251");
    std::string file_name = "dataset_task.txt";
    Parser parser(GetDataFromFile(file_name));
    parser.Parse();

    if (PutDataIntoFile("UsersDataCards.txt", parser.GetUsersData())) {
        std::cerr << "OK! You can check your file!\n";
    }
    else {
        std::cerr << "Error in putting data into file!\n";
    }
}