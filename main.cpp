#include <bits/stdc++.h>

using namespace std;

int ADDRESS_WIDTH;
int CACHE_SIZE;
int BLOCK_SIZE;
int ASSOCIATIVITY;
int NUM_SETS;
int OFFSET_WIDTH;
int INDEX_WIDTH;
int TAG_WIDTH;

int hex_to_int(string hex) {
    int x;
    stringstream ss;
    ss << std::hex << hex;
    ss >> x;
    return x;
}

string int_to_hex(int x) {
    stringstream ss;
    ss << std::hex << x;
    return ss.str();
}

void mem_init(map<string, vector<string>>& mem) {
    for (int i = 0; i < 4096; i += 8) {
        vector<string> data;
        for (int j = 0; j < 8; j++) {
            data.push_back(int_to_hex(rand() % 255));
        }
        mem[int_to_hex(i)] = data;
    }
}

void print_mem(map<string, vector<string>>& mem) {
    for (int i = 0; i < 4096; i += 8) {
        cout << "0x" << int_to_hex(i) << " ";
        for (int j = 0; j < 8; j++) {
            cout << mem[int_to_hex(i)][j] << " ";
        }
        cout << endl;
    }
}

vector<string> retrieve_from_mem(map<string, vector<string>>& mem,
                                 string hex_address) {
    int int_address = hex_to_int(hex_address);
    int x = int_address / 8;
    int y = (int_address % 8) - (int_address % BLOCK_SIZE);

    vector<string> data;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        data.push_back(mem[int_to_hex(x * 8)][y + i]);
    }
    return data;
}

map<int, vector<tuple<int, int, vector<string>>>> cache_init() {
    NUM_SETS = (CACHE_SIZE / BLOCK_SIZE) / ASSOCIATIVITY;

    OFFSET_WIDTH = log2(BLOCK_SIZE);
    INDEX_WIDTH = log2(NUM_SETS);
    TAG_WIDTH = ADDRESS_WIDTH - OFFSET_WIDTH - INDEX_WIDTH;

    cout << "Offset width is " << OFFSET_WIDTH << "\nIndex width is "
         << INDEX_WIDTH << "\nTag width is " << TAG_WIDTH << endl;

    map<int, vector<tuple<int, int, vector<string>>>> cache_mem;
    for (int i = 0; i < NUM_SETS; i++) {
        for (int j = 0; j < ASSOCIATIVITY; j++) {
            cache_mem[i].push_back(
                make_tuple(-1, -1, vector<string>(BLOCK_SIZE, "-1")));
        }
    }
    return cache_mem;
}

void retrieve_from_cache(
    map<int, vector<tuple<int, int, vector<string>>>>& cache,
    map<string, vector<string>>& main_memory, string target_address) {
    int target_address_int = hex_to_int(target_address);
    int offset = target_address_int % BLOCK_SIZE;

    int index = (target_address_int / BLOCK_SIZE) % NUM_SETS;

    int tag = target_address_int / (int)pow(2, INDEX_WIDTH + OFFSET_WIDTH);
    
    // get<0> = Tag get<1> = Dirty get<2> = Data Vector
    int i = 0;
    for (auto x : cache[index]) {
        if (get<0>(x) == -1) {
            cout << "Cache miss. Retrieving and loading from main memory"
                 << endl;
            cache[index][i] = make_tuple(
                tag, 0, retrieve_from_mem(main_memory, target_address));
            return;
        } else if (get<0>(x) == tag) {
            cout<<"Read hit"<<endl;
            cout<<"Data at 0x"<<target_address<<" is "<<get<2>(x)[offset]<<endl;
            return;
        }
        i++;
    }
    cout << endl;
    cout << "Cache miss. Retrieving and loading from main memory" << endl;
    cache[index][rand() % ASSOCIATIVITY] =
        make_tuple(tag, 0, retrieve_from_mem(main_memory, target_address));
}

int main() {
    map<string, vector<string>> main_memory;
    mem_init(main_memory);

    cout << "Enter address width: ";
    cin >> ADDRESS_WIDTH;

    cout << "Enter cache size: ";
    cin >> CACHE_SIZE;

    cout << "Enter block size: ";
    cin >> BLOCK_SIZE;

    cout << "Enter ASSOCIATIVITY: ";
    cin >> ASSOCIATIVITY;

    map<int, vector<tuple<int, int, vector<string>>>> cache_mem = cache_init();

    print_mem(main_memory);
    while (true) {
        string target_address;

        cout << "Enter address to read ";
        cin >> target_address;

        if (target_address == "q") {
            break;
        }
        retrieve_from_cache(cache_mem, main_memory, target_address);
    }
    cout << "pass" << endl;
}
