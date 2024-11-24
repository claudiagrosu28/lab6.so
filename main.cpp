#include <iostream>
#include <windows.h>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>

using namespace std;

bool isPrime(int n) {
    if (n < 2) return false;
    for (int i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) return false;
    }
    return true;
}

void childProcess(int start, int end, HANDLE hWrite) {
    vector<int> childPrimes;

    for (int n = start; n <= end; n++) {
        if (isPrime(n)) {
            childPrimes.push_back(n);
        }
    }

    for (int prime : childPrimes) {
        DWORD bytesWritten;
        WriteFile(hWrite, &prime, sizeof(prime), &bytesWritten, nullptr);
    }

    CloseHandle(hWrite);
    ExitProcess(0);
}

int main(int argc, char* argv[]) {
    if (argc == 4) {
        int start = atoi(argv[1]);
        int end = atoi(argv[2]);
        HANDLE hWrite = (HANDLE)strtoull(argv[3], nullptr, 0);
        childProcess(start, end, hWrite);
    }

    const int RANGE = 10000;
    const int NUM_PROCESSES = 10;
    const int STEP = RANGE / NUM_PROCESSES;
    vector<int> primes;

    HANDLE pipes[NUM_PROCESSES][2];
    PROCESS_INFORMATION processInfo[NUM_PROCESSES];

    for (int i = 0; i < NUM_PROCESSES; i++) {
        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };

        if (!CreatePipe(&pipes[i][0], &pipes[i][1], &sa, 0)) {
            cerr << "Error creating pipe" << endl;
            return 1;
        }

        STARTUPINFO si = {};
        si.cb = sizeof(STARTUPINFO);
        si.hStdOutput = pipes[i][1];
        si.dwFlags |= STARTF_USESTDHANDLES;

        int start = i * STEP + 1;
        int end = (i + 1) * STEP;

        std::string command = std::string(argv[0]) + " " + to_string(start) + " " + to_string(end) + " " + to_string((uintptr_t)pipes[i][1]);

        int size_needed = MultiByteToWideChar(CP_UTF8, 0, command.c_str(), (int)command.size(), nullptr, 0);
        wchar_t* wcommand = new wchar_t[size_needed + 1];
        MultiByteToWideChar(CP_UTF8, 0, command.c_str(), (int)command.size(), wcommand, size_needed);
        wcommand[size_needed] = 0;

        if (!CreateProcess(nullptr, wcommand, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &processInfo[i])) {
            cerr << "Error creating process" << endl;
            delete[] wcommand;
            return 1;
        }

        delete[] wcommand;
        CloseHandle(pipes[i][1]);
    }

    for (int i = 0; i < NUM_PROCESSES; i++) {
        WaitForSingleObject(processInfo[i].hProcess, INFINITE);

        int prime;
        DWORD bytesRead;
        while (ReadFile(pipes[i][0], &prime, sizeof(prime), &bytesRead, nullptr) && bytesRead > 0) {
            primes.push_back(prime);
        }

        CloseHandle(pipes[i][0]);
        CloseHandle(processInfo[i].hProcess);
        CloseHandle(processInfo[i].hThread);
    }

    cout << "Prime numbers up to " << RANGE << ":\n";
    for (int prime : primes) {
        cout << prime << " ";
    }
    cout << endl;

    return 0;
}
