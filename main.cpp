#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <cmath>

using namespace std;

bool isPrime(int n) {
    if (n < 2) return false;
    for (int i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) return false;
    }
    return true;
}

int main() {
    const int RANGE = 10000;
    const int NUM_PROCESSES = 10;
    const int STEP = RANGE / NUM_PROCESSES;
    int pipes[NUM_PROCESSES][2];
    vector<int> primes;

    for (int i = 0; i < NUM_PROCESSES; i++) {
        if (pipe(pipes[i]) == -1) {
            cerr << "Error creating pipe" << endl;
            return 1;
        }
    }

    for (int i = 0; i < NUM_PROCESSES; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            cerr << "Error creating process" << endl;
            return 1;
        }

        if (pid == 0) {
            // Child process
            close(pipes[i][0]); // Close read end
            int start = i * STEP + 1;
            int end = (i + 1) * STEP;
            vector<int> childPrimes;

            for (int n = start; n <= end; n++) {
                if (isPrime(n)) {
                    childPrimes.push_back(n);
                }
            }

            for (int prime : childPrimes) {
                write(pipes[i][1], &prime, sizeof(prime));
            }

            close(pipes[i][1]);
            exit(0);
        } else {
            // Parent process
            close(pipes[i][1]); // Close write end
        }
    }

    for (int i = 0; i < NUM_PROCESSES; i++) {
        wait(nullptr);
        int prime;
        while (read(pipes[i][0], &prime, sizeof(prime)) > 0) {
            primes.push_back(prime);
        }
        close(pipes[i][0]);
    }

    cout << "Prime numbers up to " << RANGE << ":\n";
    for (int prime : primes) {
        cout << prime << " ";
    }
    cout << endl;

    return 0;
}
