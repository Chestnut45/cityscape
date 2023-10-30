#pragma once

class Building
{
    // Interface
    public:
        Building();
        ~Building();

        // Delete copy constructor/assignment
        Building(const Building&) = delete;
        Building& operator=(const Building&) = delete;

        // Delete move constructor/assignment
        Building(Building&& other) = delete;
        void operator=(Building&& other) = delete;
    
    // Data / implementation
    private:
        
};