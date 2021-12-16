#ifndef INF_THEORY_LIB_H
#define INF_THEORY_LIB_H

#include <vector>

using namespace std;

static vector<uint8_t> ones{1, 2, 4, 8, 16, 32, 64, 128};


struct Node {
    size_t sum;
    Node *left;
    Node *right;
    uint8_t *id;

    bool has_two_kids() const {
        return left != nullptr && right != nullptr;
    }

    void add_child(Node *child) {
        if (left == nullptr) {
            left = child;
        } else if (right == nullptr) {
            right = child;
        } else {
            throw runtime_error("sdc");
        }
    }
//1111111110011101100101001011100110010011100100110010010000111000000111011110111100100001110100111001100100100000111101000011101110100111010011010011001001111001001100100111001100100111001001100100101101000011010001101011110000011110111111110011001000011011100100000011110100001111010011001001110010000101000110000
    Node(uint8_t *id, size_t sum, Node *left, Node *right) : id(id), sum(sum), left(left), right(right) {}
};

struct code {
    size_t len{};
    vector<uint8_t> data;

    code(size_t len, const vector<uint8_t> &data) : len(len), data(data) {}

    code() = default;

    void add_bit(int bit) {
        if (len % 8 == 0) {
            data.push_back(0);
        }
        data[len / 8] ^= bit << len % 8;
        len++;
    }

    void remove_bit() {
        if (len % 8 == 1) {
            data.pop_back();
        }
        data[len / 8] &= 255 ^ (1 << len % 8);
        len--;
    }
};


struct bit_buffer {
    size_t pos = 0;
    size_t read_pos = 0;
    vector<uint8_t> data;

    void add_bits(const code &c) {
        for (size_t i = 0; i < c.len; ++i) {
            if (pos % 8 == 0) {
                data.push_back(0);
            }
            data[pos / 8] ^= (((c.data[i / 8] & ones[i % 8]) >> (i % 8)) << (pos % 8));
            pos++;
        }
    }

    void write_bit(int bit) {
        add_bits(code(1, {static_cast<unsigned char>(bit)}));
    }

    int read_bit() {
        int res = (data[read_pos / 8] & (1 << (read_pos % 8))) == 0 ? 0 : 1;
        read_pos++;
        return res;
    }
};

void print_tree(Node *root) {
    if (root->left != nullptr && root->right != nullptr) {
        cout << 1;
        print_tree(root->left);
        print_tree(root->right);
    } else {
        cout << 0;
    }
}

string convertToString(const vector<uint8_t> &data) {
    string a;
    for (unsigned char i: data) {
        a.push_back((char) i);
    }
    return a;
}


string convertNumbersToString(const vector<uint8_t> &data) {
    string a;
    for (unsigned char i: data) {
        a.append(to_string(i)).append(" ");
    }
    return a;
}

vector<uint8_t> convertToUintVector(const string &data) {
    vector<uint8_t> a;
    for (unsigned char i: data) {
        a.push_back((char) i);
    }
    return a;
}

void print_shifts(const vector<uint8_t> &data, size_t len, vector<size_t> &shifts) {
    cout << "SHIFTS\n";
    for (const auto &shift: shifts) {
        string a;
        for (int i = 0; i < len; ++i) {
            a.push_back((char) data[(i + shift) % len]);
        }
        cout << a << endl;
    }
}

const int numOfChars = 256;
const int numOfSyms = 257;
const int bits = 16;
const int max_bits = (1 << bits) - 1;
const int q1 = max_bits / 4 + 1;
const int q2 = q1 * 2;
const int q3 = q1 * 3;
const int max_freq = q1 - 1;


struct Model {
    vector<int> chToI = vector<int>(numOfChars, 0);
    vector<int> iToCh = vector<int>(numOfSyms + 1, 0);
    vector<int> freq = vector<int>(numOfSyms + 1, 0);
    vector<int> cum = vector<int>(numOfSyms + 1, 0);

    Model() {
        for (int i = 0; i < numOfChars; ++i) {
            chToI[i] = i + 1;
            iToCh[i + 1] = i;
        }
        for (int i = 0; i <= numOfSyms; ++i) {
            freq[i] = 1;
            cum[i] = numOfSyms - i;
        }
        freq[0] = 0;
    }

    void fitFreq() {
        if (cum[0] == max_freq) {
            int tmp = 0;
            for (int i = numOfSyms; i >= 0; --i) {
                freq[i] = (freq[i] + 1) / 2;
                cum[i] = tmp;
                tmp += freq[i];
            }
        }
    }

    int moveSymbol(int index) {
        int newIndex = index;
        while (freq[newIndex] == freq[newIndex - 1]) {
            --newIndex;
        }
        if (newIndex < index) {
//            swap(iToCh[newIndex], iToCh[index]);
            int newCh = iToCh[newIndex];
            int oldCh = iToCh[index];
            iToCh[newIndex] = oldCh;
            iToCh[index] = newCh;
            chToI[newCh] = index;
            chToI[oldCh] = newIndex;
        }
        return newIndex;
    }

    void updateFreq(int index) {
        freq[index]++;
        int tmp = index;
        while (tmp > 0) {
            --tmp;
            cum[tmp] += 1;
        }
    }

    void updateModel(int symbolIndex) {
        fitFreq();
        int index = moveSymbol(symbolIndex);
        updateFreq(index);
    }
};



#endif //INF_THEORY_LIB_H
