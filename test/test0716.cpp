#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Forward declaration
class Office;

// Employee class
class Employee {
public:
	Employee() {}
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

    void addedEmployee(std::string n) {
		for (int j = 0; j < 3; j++) {
            std::string employee_name = n + "_emp_" + std::to_string(j);
        	Employee newEmployee(employee_name, j);
        	newEmployee.office = this;
        	employees.push_back(newEmployee);
        }
        return;
    }
};

// Company class
class Company {
public:
    void setAllOffices() {
        for (int i = 0; i < 3; i++) {
            std::string office_addr(1, 'G' + i);
            auto tempOffice = std::make_unique<Office>(office_addr);

            tempOffice->addedEmployee(office_addr);

			for (auto &e : tempOffice->employees) {
            	allEmployees.push_back(e);
			}

            allOffices.push_back(std::move(tempOffice));
        }
    }

    void addOffice(std::unique_ptr<Office> office) {
        allOffices.push_back(std::move(office));
    }

    std::vector<std::unique_ptr<Office>> allOffices;
    std::vector<Employee> allEmployees;
};

using namespace std;

// Main function
int main() {
    Company company;
    company.setAllOffices();

	for (auto e : company.allEmployees) {
		cout << e.office->address << ", " << e.name << endl;
	}
	return 0;

	unique_ptr<Employee> b1 = make_unique<Employee>("test", 30);
	unique_ptr<Employee> b2 = make_unique<Employee>("test", 30);
	unique_ptr<Employee> b3 = make_unique<Employee>("test", 30);
	Employee *b_copy1 = b1.get();
	Employee &b_copy2 = *b2;
	Employee b_copy3 = *b3;
	return 0;

	for (auto const &e : company.allOffices[0]->employees){
		cout << e.name << endl;
	}
	return 0;

	Employee emp = company.allEmployees[0];
	emp.office->address = "hihi";
	cout << company.allOffices[0]->address;
	return 0;

	Employee temp = company.allOffices[0]->employees[0];
	temp.office->address = "Haha_changed";
	cout << company.allOffices[0]->address;
	return 0;

	unique_ptr<Office> shrPtrOff1 = make_unique<Office>("a");
	shrPtrOff1->addedEmployee("Austin");

	Employee *shrPtrEmp1 = &shrPtrOff1->employees[0];

	shrPtrEmp1->ID = 35;

	cout << shrPtrOff1->employees[0].ID << endl;

	cout << shrPtrEmp1->ID << endl;

	cout << "\ndone.";
    return 0;
}