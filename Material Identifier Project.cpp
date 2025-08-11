// Dielectric Constant Simulation Project
// Developed for B.Tech 1st Year PBL
// Topic: Variation of Dielectric Constant with Temperature and Curie Temperature

#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <limits>
#include <algorithm> // Added for sort function

using namespace std;

// Constants
const double epsilon_0 = 8.85e-12; // F/m (Permittivity of free space) - corrected value
// Converting to pF/mm: 8.85e-12 F/m = 8.85e-3 pF/mm

// Sample structure
struct Sample {
    string name;
    double area_mm2;
    double thickness_mm;
    double curie_temp_C;
    vector<pair<int, double>> temp_capacitance_data;
};

// Function declarations
void showTheory();
void showApparatus();
void showProcedure();
void showPrecautions();
void inputReadings(Sample &sample);
void calculateDielectricConstants(Sample &sample);
void displayGraph(Sample &sample);
void analyzeCurieTemperature(Sample &sample);
void saveToFile(Sample &sample);
void simulate();
void clearInputBuffer();

// Materials database
map<string, Sample> materials = {
    {"Barium Titanate", {"Barium Titanate", 8 * 6, 1.42, 120, {}}},
    {"Titanium Dioxide", {"Titanium Dioxide", 8 * 6, 1.42, 50, {}}},
    {"Quartz", {"Quartz", 8 * 6, 1.42, -1, {}}}
};

int main() {
    int choice;
    do {
        cout << "\n===== Dielectric Constant and Curie Temperature Simulation =====\n";
        cout << "1. Show Theory\n";
        cout << "2. Show Apparatus\n";
        cout << "3. Show Procedure\n";
        cout << "4. Show Precautions\n";
        cout << "5. Start Simulation\n";
        cout << "6. Exit\n";
        cout << "Enter your choice: ";
        
        // Improved input handling
        if (!(cin >> choice)) {
            cout << "Invalid input. Please enter a number.\n";
            clearInputBuffer();
            continue;
        }

        switch (choice) {
            case 1: showTheory(); break;
            case 2: showApparatus(); break;
            case 3: showProcedure(); break;
            case 4: showPrecautions(); break;
            case 5: simulate(); break;
            case 6: cout << "Exiting program.\n"; break;
            default: cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 6);

    return 0;
}

void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void showTheory() {
    cout << "\n--- THEORY ---\n";
    cout << "Dielectric materials are insulating substances where electrostatic fields persist.\n";
    cout << "The dielectric constant (ε) is the ratio of capacitance with and without the dielectric.\n";
    cout << "For materials like BaTiO3, ε increases as temperature increases, peaking at Curie temperature.\n";
    cout << "After Curie temperature, ferroelectricity is lost, and ε decreases.\n\n";
}

void showApparatus() {
    cout << "\n--- APPARATUS USED ---\n";
    cout << "1. Barium Titanate (BaTiO3) Sample\n";
    cout << "2. Oven with temperature controller\n";
    cout << "3. Digital capacitance meter\n";
    cout << "4. RTD sensor for temperature\n";
    cout << "5. Probe arrangement with aluminum foil\n\n";
}

void showProcedure() {
    cout << "\n--- PROCEDURE ---\n";
    cout << "1. Mount the sample with probes and aluminum foil\n";
    cout << "2. Connect probes to the capacitance meter\n";
    cout << "3. Heat the sample in oven\n";
    cout << "4. Measure capacitance at different temperatures\n";
    cout << "5. Calculate ε using ε = C / C0, where C0 = ε0*A/t\n\n";
}

void showPrecautions() {
    cout << "\n--- PRECAUTIONS ---\n";
    cout << "1. Probe should touch sample gently.\n";
    cout << "2. Take small intervals near Curie temperature.\n";
    cout << "3. Take reading only when oven is OFF.\n\n";
}

void simulate() {
    cout << "\nAvailable materials:\n";
    int index = 1;
    vector<string> keys;
    for (map<string, Sample>::iterator it = materials.begin(); it != materials.end(); ++it) {
        cout << index++ << ". " << it->first << endl;
        keys.push_back(it->first);
    }
    
    int material_choice;
    cout << "Select a material (1-" << keys.size() << "): ";
    
    // Improved input validation
    while (!(cin >> material_choice) || material_choice < 1 || material_choice > static_cast<int>(keys.size())) {
        cout << "Invalid selection. Please enter a number between 1 and " << keys.size() << ": ";
        clearInputBuffer();
    }
    
    // Safe access to the selected material
    Sample sample = materials[keys[material_choice - 1]];
    
    // Clear any existing data (if reusing the sample)
    sample.temp_capacitance_data.clear();

    inputReadings(sample);
    
    // Only proceed if we have data
    if (!sample.temp_capacitance_data.empty()) {
        calculateDielectricConstants(sample);
        
        // Only analyze Curie temperature for ferroelectric materials
        if (sample.curie_temp_C > 0) {
            analyzeCurieTemperature(sample);
        } else {
            cout << "\nNote: This material doesn't have a Curie temperature (non-ferroelectric).\n";
        }
        
        displayGraph(sample);
        saveToFile(sample);
    } else {
        cout << "\nNo data entered. Returning to main menu.\n";
    }
}

void inputReadings(Sample &sample) {
    cout << "\nEnter temperature (°C) and capacitance (pF). Type -1 for temperature to stop.\n";
    int temp;
    double capacitance;
    
    while (true) {
        cout << "Temperature (°C): ";
        if (!(cin >> temp)) {
            cout << "Invalid input. Please enter a number.\n";
            clearInputBuffer();
            continue;
        }
        
        if (temp == -1) break;
        
        cout << "Capacitance (pF): ";
        if (!(cin >> capacitance)) {
            cout << "Invalid input. Please enter a number.\n";
            clearInputBuffer();
            continue;
        }
        
        // Validate the data (basic sanity check)
        if (temp < -273 || capacitance <= 0) {
            cout << "Invalid values. Temperature must be above -273°C and capacitance must be positive.\n";
            continue;
        }
        
        sample.temp_capacitance_data.push_back(make_pair(temp, capacitance));
    }
    
    // Sort data by temperature (ascending) for better display
    sort(sample.temp_capacitance_data.begin(), sample.temp_capacitance_data.end());
}

void calculateDielectricConstants(Sample &sample) {
    // Calculate the capacitance of equivalent vacuum capacitor C0 = ε0*A/t
    double C0 = epsilon_0 * 1e12 * (sample.area_mm2 / sample.thickness_mm); // Convert to pF
    
    cout << fixed << setprecision(2);
    cout << "\n------ RESULTS ------\n";
    cout << "Material: " << sample.name << "\n";
    cout << "Sample dimensions: " << sqrt(sample.area_mm2) << " mm × " << sqrt(sample.area_mm2) << " mm × " << sample.thickness_mm << " mm\n";
    cout << "Vacuum capacitance (C0): " << C0 << " pF\n\n";
    
    cout << "Temp (°C)\tCapacitance (pF)\tDielectric Constant (ε)\n";
    cout << "--------------------------------------------------------\n";
    
    for (size_t i = 0; i < sample.temp_capacitance_data.size(); i++) {
        int temp = sample.temp_capacitance_data[i].first;
        double C = sample.temp_capacitance_data[i].second;
        double epsilon = C / C0;
        cout << temp << "\t\t" << C << "\t\t" << epsilon << endl;
    }
}

void analyzeCurieTemperature(Sample &sample) {
    if (sample.temp_capacitance_data.size() < 2) {
        cout << "\nNot enough data points to estimate Curie temperature.\n";
        return;
    }
    
    double max_epsilon = 0;
    int max_temp = 0;
    double C0 = epsilon_0 * 1e12 * (sample.area_mm2 / sample.thickness_mm); // Convert to pF
    
    for (size_t i = 0; i < sample.temp_capacitance_data.size(); i++) {
        int temp = sample.temp_capacitance_data[i].first;
        double C = sample.temp_capacitance_data[i].second;
        double epsilon = C / C0;
        if (epsilon > max_epsilon) {
            max_epsilon = epsilon;
            max_temp = temp;
        }
    }
    
    cout << "\nEstimated Curie Temperature: " << max_temp << "°C\n";
    cout << "Expected Curie Temperature for " << sample.name << ": " << sample.curie_temp_C << "°C\n";
    cout << "Difference: " << abs(max_temp - sample.curie_temp_C) << "°C\n";
}

void displayGraph(Sample &sample) {
    if (sample.temp_capacitance_data.empty()) {
        cout << "\nNo data to display graph.\n";
        return;
    }
    
    cout << "\nASCII Graph: Dielectric Constant vs Temperature\n";
    cout << "-----------------------------------------------\n";
    
    double C0 = epsilon_0 * 1e12 * (sample.area_mm2 / sample.thickness_mm); // Convert to pF
    
    // Find the maximum epsilon for scaling
    double max_epsilon = 0;
    for (size_t i = 0; i < sample.temp_capacitance_data.size(); i++) {
        double C = sample.temp_capacitance_data[i].second;
        double epsilon = C / C0;
        max_epsilon = max(max_epsilon, epsilon);
    }
    
    // Scale factor (adjusting number of bars)
    double scale = 50.0 / max_epsilon;
    
    for (size_t i = 0; i < sample.temp_capacitance_data.size(); i++) {
        int temp = sample.temp_capacitance_data[i].first;
        double C = sample.temp_capacitance_data[i].second;
        double epsilon = C / C0;
        cout << setw(4) << temp << "°C | ";
        int bars = static_cast<int>(epsilon * scale);
        for (int j = 0; j < bars; ++j) cout << "#";
        cout << " (" << fixed << setprecision(2) << epsilon << ")\n";
    }
}

void saveToFile(Sample &sample) {
    string filename = sample.name + "_results.txt";
    // Remove spaces from filename
    for (size_t i = 0; i < filename.length(); i++) {
        if (filename[i] == ' ') filename[i] = '_';
    }
    
    ofstream file(filename.c_str());  // Using c_str() for older compilers
    if (!file.is_open()) {
        cout << "\nError: Could not create file for saving results.\n";
        return;
    }
    
    double C0 = epsilon_0 * 1e12 * (sample.area_mm2 / sample.thickness_mm); // Convert to pF
    
    file << "Dielectric Constant Measurement Results\n";
    file << "Material: " << sample.name << "\n";
    file << "Sample dimensions: " << sqrt(sample.area_mm2) << " mm × " << sqrt(sample.area_mm2) << " mm × " 
         << sample.thickness_mm << " mm\n";
    file << "Vacuum capacitance (C0): " << C0 << " pF\n\n";
    
    file << "Temperature (°C)\tCapacitance (pF)\tDielectric Constant (ε)\n";
    file << "--------------------------------------------------------\n";
    
    for (size_t i = 0; i < sample.temp_capacitance_data.size(); i++) {
        int temp = sample.temp_capacitance_data[i].first;
        double C = sample.temp_capacitance_data[i].second;
        double epsilon = C / C0;
        file << temp << "\t\t" << C << "\t\t" << epsilon << "\n";
    }
    
    file.close();
    cout << "\nResults saved to '" << filename << "'.\n";
}