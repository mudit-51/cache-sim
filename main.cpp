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
int WRITE_HIT;
int WRITE_MISS;

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

void write_to_mem(map<string, vector<string>>& mem, string hex_address,
                  string data) {
    int int_address = hex_to_int(hex_address);
    int x = int_address / 8;
    int y = int_address % 8;
    mem[int_to_hex(x * 8)][y] = data;
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

void print_cache(map<int, vector<tuple<int, int, vector<string>>>>& cache) {
    for (int i = 0; i < (int)pow(2, INDEX_WIDTH); i++) {
        cout << "Index " << i << endl;
        for (auto x : cache[i]) {
            int tag, dirty;
            vector<string> data;
            tie(tag, dirty, data) = x;
            cout << tag << " " << dirty << " ";
            for (auto y : data) {
                cout << y << " ";
            }
            cout << endl;
        }
        cout << endl;
    }
}
tuple<int, int, vector<string>>* read_from_cache(
    map<int, vector<tuple<int, int, vector<string>>>>& cache,
    map<string, vector<string>>& main_memory, string target_address) {
    int target_address_int = hex_to_int(target_address);
    int offset = target_address_int % BLOCK_SIZE;

    int index = (target_address_int / BLOCK_SIZE) % NUM_SETS;

    int tag = target_address_int / (int)pow(2, INDEX_WIDTH + OFFSET_WIDTH);

    // get<0> = Tag | get<1> = Dirty | get<2> = Data Vector
    int i = 0;
    for (auto x : cache[index]) {
        if (get<0>(x) == -1) {
            cout << "Cache miss. Retrieving and loading from main memory"
                 << endl;
            cache[index][i] = make_tuple(
                tag, 0, retrieve_from_mem(main_memory, target_address));
            return &cache[index][i];
        } else if (get<0>(x) == tag) {
            cout << "Read hit" << endl;
            cout << "Data at 0x" << target_address << " is "
                 << get<2>(x)[offset] << endl;
            return &cache[index][i];
        }
        i++;
    }
    return NULL;
}

tuple<int, int, vector<string>>* read_and_evict_from_cache(
    map<int, vector<tuple<int, int, vector<string>>>>& cache,
    map<string, vector<string>>& main_memory, string target_address) {
    int target_address_int = hex_to_int(target_address);
    int offset = target_address_int % BLOCK_SIZE;

    int index = (target_address_int / BLOCK_SIZE) % NUM_SETS;

    int tag = target_address_int / (int)pow(2, INDEX_WIDTH + OFFSET_WIDTH);

    auto* x = read_from_cache(cache, main_memory, target_address);

    if (x) {
        return x;
    }

    cout << endl;
    cout << "Cache full. Replacing random cache line and loading from main "
            "memory"
         << endl;

    int temp = rand() % ASSOCIATIVITY;

    auto y = cache[index][temp];

    cache[index][temp] =
        make_tuple(tag, 0, retrieve_from_mem(main_memory, target_address));

    int old_address = get<0>(y) * (int)pow(2, INDEX_WIDTH + OFFSET_WIDTH) +
                      index * (int)pow(2, INDEX_WIDTH);

    for (int i = 0; i < BLOCK_SIZE; i++) {
        write_to_mem(main_memory, int_to_hex(old_address + i), get<2>(y)[i]);
    }

    return &cache[index][temp];
}

void write_through(map<int, vector<tuple<int, int, vector<string>>>>& cache,
                   map<string, vector<string>>& main_memory,
                   string target_address, string new_data) {
    int target_address_int = hex_to_int(target_address);
    int offset = target_address_int % BLOCK_SIZE;

    int index = (target_address_int / BLOCK_SIZE) % NUM_SETS;

    int tag = target_address_int / (int)pow(2, INDEX_WIDTH + OFFSET_WIDTH);

    if (WRITE_MISS) {
        auto* x = read_and_evict_from_cache(cache, main_memory, target_address);
        get<2>(*x)[offset] = new_data;
        get<1>(*x) = 1;
    } else {
        auto* x = read_from_cache(cache, main_memory, target_address);
        if (x) {
            get<2>(*x)[offset] = new_data;
            get<1>(*x) = 1;
        }
    }
    write_to_mem(main_memory, target_address, new_data);
    cout << endl;
    print_cache(cache);
    cout << endl;
}

void write_back(map<int, vector<tuple<int, int, vector<string>>>>& cache,
                map<string, vector<string>>& main_memory, string target_address,
                string new_data) {
    int target_address_int = hex_to_int(target_address);
    int offset = target_address_int % BLOCK_SIZE;

    int index = (target_address_int / BLOCK_SIZE) % NUM_SETS;

    int tag = target_address_int / (int)pow(2, INDEX_WIDTH + OFFSET_WIDTH);

    if (WRITE_MISS) {
        auto* x = read_and_evict_from_cache(cache, main_memory, target_address);

        get<2>(*x)[offset] = new_data;
        get<1>(*x) = 1;
    } else {
        auto* x = read_from_cache(cache, main_memory, target_address);
        if (x) {
            get<2>(*x)[offset] = new_data;
            get<1>(*x) = 1;
        } else {
            write_to_mem(main_memory, target_address, new_data);
        }
    }
    cout << endl;
    print_cache(cache);
    cout << endl;
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

    cout << "Write back(0) / Write through (1): ";
    cin >> WRITE_HIT;

    cout << "Allocate (1) / No Allocate (0): ";
    cin >> WRITE_MISS;

    map<int, vector<tuple<int, int, vector<string>>>> cache_mem = cache_init();

    print_mem(main_memory);
    while (true) {
        string target_address;

        cout << "Enter address to read ";
        cin >> target_address;

        if (target_address == "q") {
            break;
        }
        read_and_evict_from_cache(cache_mem, main_memory, target_address);
    }
    print_cache(cache_mem);
    while (true) {
        string target_address;
        string input_byte;

        cout << "Enter address to write ";
        cin >> target_address;

        cout << "Enter byte value to write (in hex): ";
        cin >> input_byte;

        if (target_address == "q") {
            break;
        }
        if (WRITE_HIT) {
            write_through(cache_mem, main_memory, target_address, input_byte);
        } else {
            write_back(cache_mem, main_memory, target_address, input_byte);
        }
    }
    cout << "pass" << endl;
}
