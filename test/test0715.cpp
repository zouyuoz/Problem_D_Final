#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Forward declaration
class Office;

// Employee class
class Employee {
public:
    Employee(std::string n, int i) : name(n), ID(i), office(nullptr) {}
    std::string name;
    int ID;
    Office* office;
};

// Office class
class Office {
public:
    Office(std::string addr) : address(addr) {}
    std::string address;
    std::vector<Employee> employees;

    Employee addedEmployee(std::string n, int i) {
        Employee newEmployee(n, i);
        newEmployee.office = this;
        employees.push_back(newEmployee);
        return newEmployee;
    }
};

// Company class
class Company {
public:
    void setAllOffices() {
        for (int i = 0; i < 3; i++) {
            std::string office_addr(1, 'G' + i);
            auto tempOffice = std::make_unique<Office>(office_addr);
            for (int j = 0; j < 3; j++) {
                std::string employee_name = office_addr + "_emp_" + std::to_string(j);
                auto newEmployee = tempOffice->addedEmployee(employee_name, j);
                allEmployees.push_back(std::make_unique<Employee>(std::move(newEmployee)));
            }
            addOffice(std::move(tempOffice));
        }
    }

    void addOffice(std::unique_ptr<Office> office) {
        allOffices.push_back(std::move(office));
    }

    std::vector<std::unique_ptr<Office>> allOffices;
    std::vector<std::unique_ptr<Employee>> allEmployees;
};

using namespace std;

// Main function
int main() {
    Company company;
    company.setAllOffices();

    for (const auto& emp : company.allEmployees) {
        int getID = emp->ID;
		cout << getID << endl;
    }


    return 0;
}