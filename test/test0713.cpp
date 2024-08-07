#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Employee {
public:
	Employee(string n, int i) : name(n), ID(i) {}
	string name;
	int ID;
};

class Office {
public:
	Office() {}
	vector<Employee> employees;
	string address;
};

int main() {
	Office office_a;
	office_a.employees.push_back(Employee("Andy", 0));
	office_a.employees.push_back(Employee("Amber", 1));
	office_a.employees.push_back(Employee("Austin", 2));

	Office office_b;
	office_b.employees.push_back(Employee("Ben", 0));
	office_b.employees.push_back(Employee("Bruce", 1));
	office_b.employees.push_back(Employee("Brian", 2));

	vector<Employee> allEmployees;
	for (Employee &e : office_a.employees) { allEmployees.push_back(e); }
	for (Employee &e : office_b.employees) { allEmployees.push_back(e); }

	cout << "\ndone";
	return 0;
}