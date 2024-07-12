#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <regex>
#include <windows.h> // Für GetLogicalDrives
#include <atlsecurity.h> // HWID

using namespace std;
namespace fs = filesystem;

const string profilesDir = "C:/Scotch/altV Profiles";

// Funktion zum Prüfen und Erstellen des Profilordners
void checkAndCreateProfilesDir() {
    if (!fs::exists(profilesDir)) {
        fs::create_directories(profilesDir);
    }
}

// Funktion zum Suchen der altv.toml Datei auf allen Laufwerken
string findAltvToml() {
    DWORD drives = GetLogicalDrives();
    char driveLetter = 'A';

    for (DWORD i = 0; i < 26; ++i) {
        if (drives & (1 << i)) {
            string drive = string(1, driveLetter + i) + ":/";

            try {
                for (const auto& entry : fs::recursive_directory_iterator(drive)) {
                    if (entry.is_regular_file() && entry.path().filename() == "altv.toml") {
                        return entry.path().string();
                    }
                }
            }
            catch (const fs::filesystem_error& ex) {
                cerr << "Error accessing directory: " << ex.what() << endl;
            }
        }
    }

    return ""; // Return empty string if altv.toml is not found
}
// Funktion zum Laden der Profile
vector<string> loadProfiles() {
    vector<string> profiles;
    for (const auto& entry : fs::directory_iterator(profilesDir)) {
        if (entry.path().extension() == ".txt") {
            ifstream profileFile(entry.path().string());
            string firstLine;
            getline(profileFile, firstLine);
            if (firstLine == "Scotch altV Changer") {
                profiles.push_back(entry.path().stem().string());
            }
        }
    }
    sort(profiles.begin(), profiles.end());
    return profiles;
}

// Funktion zum Anzeigen der Profiloptionen
void displayProfileOptions(const vector<string>& profiles) {
    for (size_t i = 0; i < profiles.size(); ++i) {
        cout << i + 1 << ") " << profiles[i] << endl;
    }
    cout << profiles.size() + 1 << ") Neues Profil erstellen" << endl;
}

// Funktion zum Bearbeiten der altv.toml Datei
void modifyTomlFile(const string& filePath, const string& newName) {
    ifstream inputFile(filePath);
    if (!inputFile) {
        cerr << "Fehler beim Öffnen der Datei!" << endl;
        return;
    }

    stringstream buffer;
    string line;
    bool nameModified = false;

    // Jede Zeile der Datei durchgehen
    while (getline(inputFile, line)) {
        // Wenn die Zeile mit "name =" beginnt, modifizieren
        if (line.find("name =") == 0) {
            buffer << "name = '" << newName << "'" << endl;
            nameModified = true;
        }
        else {
            buffer << line << endl;
        }
    }

    inputFile.close();

    if (!nameModified) {
        cerr << "Die Zeile mit 'name =' wurde nicht gefunden!" << endl;
        return;
    }

    // Modifizierten Inhalt in die Datei zurückschreiben
    ofstream outputFile(filePath);
    if (!outputFile) {
        cerr << "Fehler beim Schreiben in die Datei!" << endl;
        return;
    }

    outputFile << buffer.str();
    outputFile.close();
}

// Funktion zum Laden eines Profils
void loadProfile(const string& profileName, const string& altvTomlPath) {
    string profilePath = profilesDir + "/" + profileName + ".txt";
    ifstream profileFile(profilePath);
    string line;
    while (getline(profileFile, line)) {
        if (line.find("Dein Name =") == 0) {
            string newName = line.substr(line.find('=') + 2);
            modifyTomlFile(altvTomlPath, newName);
            break;
        }
    }
    MessageBoxA(0, "Profil wurde geladen", "ERFOLGREICH", MB_ICONINFORMATION);
}

// Funktion zum Editieren eines Profils
void editProfile(const string& profileName) {
    string profilePath = profilesDir + "/" + profileName + ".txt";
    ifstream profileFile(profilePath);
    string firstLine, secondLine;
    getline(profileFile, firstLine);
    getline(profileFile, secondLine);
    profileFile.close();

    cout << "Geben Sie den neuen Namen für das Profil ein: ";
    string newName;
    getline(cin, newName);

    ofstream outputProfileFile(profilePath);
    outputProfileFile << firstLine << endl;
    outputProfileFile << "Dein Name = " << newName << endl;
    outputProfileFile.close();

    cout << "Möchten Sie den Dateinamen ändern? (y/n): ";
    char choice;
    cin >> choice;
    cin.ignore();
    if (choice == 'y' || choice == 'Y') {
        cout << "Geben Sie den neuen Dateinamen ein: ";
        string newFileName;
        getline(cin, newFileName);
        fs::rename(profilePath, profilesDir + "/" + newFileName + ".txt");
    }
    cout << "Profil wurde aktualisiert." << endl;
}

// Funktion zum Löschen eines Profils
void deleteProfile(const string& profileName) {
    string profilePath = profilesDir + "/" + profileName + ".txt";
    fs::remove(profilePath);
    cout << "Profil " << profileName << " wurde gelöscht." << endl;
}

void createNewProfile(const string& profileName, const string& altvTomlPath) {
    // Erstelle die Datei für das neue Profil im Profilverzeichnis
    ofstream profileFile("C:/Scotch/altV Profiles/" + profileName + ".txt");
    if (!profileFile.is_open()) {
        cerr << "Konnte Profildatei nicht erstellen: " << profileName << endl;
        return;
    }

    // Schreibe die erforderlichen Zeilen in die Profildatei
    profileFile << "Scotch altV Changer\n";
    profileFile << "name = '" + profileName + "'\n";

    profileFile.close();
}



// Funktion zur Überprüfung der Hardware-ID
string GetHWID() {
    HW_PROFILE_INFO hwProfileInfo;
    GetCurrentHwProfile(&hwProfileInfo);
    wstring hwidWString = hwProfileInfo.szHwProfileGuid;
    return string(hwidWString.begin(), hwidWString.end());
}

static string get_hwid()
{
    ATL::CAccessToken accessToken;
    ATL::CSid currentUserSid;
    if (accessToken.GetProcessToken(TOKEN_READ | TOKEN_QUERY) &&
        accessToken.GetUser(&currentUserSid))
        return std::string(CT2A(currentUserSid.Sid()));
}


// Hauptfunktion
int main() {
    SetConsoleTitleA("alt:V Name Changer by scotch5627");
    // Entscheidung basierend auf HWID und SID
    bool isValidCredentials = (GetHWID() == "{ZENSIERT}" && get_hwid() == "ZENSIERT");

    if (isValidCredentials) { // wenn meine HWID
        // Überprüfen und Erstellen des Profilordners
        checkAndCreateProfilesDir();

        // Suchen der altv.toml Datei
        string altvTomlPath = findAltvToml();
        if (altvTomlPath.empty()) {
            cerr << "altv.toml Datei nicht gefunden!" << endl;
            return 1;
        }

        // Laden der Profile
        vector<string> profiles = loadProfiles();
        // Wenn keine Profile gefunden wurden, direkt ein neues erstellen
        if (profiles.empty()) {
            cout << "Keine Profile gefunden. Erstelle ein neues Profil." << endl;
            string profileName;
            cout << "Geben Sie den Namen für das neue Profil ein: ";
            getline(cin, profileName);

            // Erstelle ein neues Profil
            createNewProfile(profileName, altvTomlPath);

            // Füge das neue Profil zur Liste hinzu
            profiles.push_back(profileName);
        }

        // Anzeigen der verfügbaren Profile
        cout << "Verfügbare Profile:" << endl;
        displayProfileOptions(profiles);

        // Benutzer zur Auswahl eines Profils auffordern
        cout << "Wählen Sie ein Profil aus (Nummer): ";
        size_t choice;
        cin >> choice;
        cin.ignore();
        if (choice < 1 || choice > profiles.size() + 1) {
            cerr << "Ungültige Auswahl!" << endl;
            return 1;
        }

        if (choice == profiles.size() + 1) {
            // Benutzer möchte ein neues Profil erstellen
            string profileName;
            cout << "Geben Sie den Namen für das neue Profil ein: ";
            getline(cin, profileName);

            // Erstelle ein neues Profil
            createNewProfile(profileName, altvTomlPath);

            // Füge das neue Profil zur Liste hinzu
            profiles.push_back(profileName);

            // Wähle das neu erstellte Profil aus
            choice = profiles.size();
        }

        string selectedProfile = profiles[choice - 1];

        // Optionen für das ausgewählte Profil anzeigen
        cout << "1) Profil laden" << endl;
        cout << "2) Profil editieren" << endl;
        cout << "3) Profil löschen" << endl;
        cout << "Wählen Sie eine Option (Nummer): ";
        int option;
        cin >> option;
        cin.ignore();

        // Je nach Benutzerwahl entsprechende Aktion ausführen
        switch (option) {
        case 1:
            loadProfile(selectedProfile, altvTomlPath);
            break;
        case 2:
            editProfile(selectedProfile);
            break;
        case 3:
            deleteProfile(selectedProfile);
            break;
        default:
            MessageBoxA(0, "Ungültige Auswahl!", "FEHLER", MB_ICONERROR);
            return 1;
        }

        return 0;
    }
    else { // andere HWID
        MessageBoxA(0, "Falsche HWID!", "FEHLER", MB_ICONERROR);
        return 1;
    }
}