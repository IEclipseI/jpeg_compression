#include <iostream>
#include <utility>
#include <vector>
#include <unordered_map>
#include <set>
#include <map>
#include <fstream>
#include <algorithm>
#include "lib.h"


using namespace std;

pair<const vector<uint8_t>, size_t> bwt(const vector<uint8_t> &data) {
    size_t len = data.size();

    vector<size_t> indexes = vector<size_t>(len);
    for (int i = 0; i < len; ++i) {
        indexes[i] = i;
    }

    vector<uint8_t> data_cpy(data);
    for (int i = 0; i < len; ++i) {
        data_cpy.push_back(data[i]);
    }

    sort(indexes.begin(), indexes.end(), [&data_cpy, len](size_t a, size_t b) {
        for (int i = 0; i < len; ++i) {
            if (data_cpy[a + i] != data_cpy[b + i]) {
                return data_cpy[a + i] < data_cpy[b + i];
            }
        }
        return false;
    });

    vector<uint8_t> result(len);
    size_t zero_pos = 0;
    for (int i = 0; i < len; ++i) {
        result[i] = data[(indexes[i] + len - 1) % len];
        if (indexes[i] == 0) {
            zero_pos = i;
        }
    }
    return {result, zero_pos};
}


vector<uint8_t> mft(const vector<uint8_t> &input, const vector<uint8_t> &sorted_alph) {
    vector<uint8_t> current_indexes(sorted_alph.size());
    unordered_map<uint8_t, size_t> pos;

    for (size_t i = 0; i < sorted_alph.size(); ++i) {
        current_indexes[i] = sorted_alph[i];
        pos[sorted_alph[i]] = i;
    }
    vector<uint8_t> result(input.size());

    for (size_t i = 0; i < input.size(); ++i) {
        uint8_t cur = input[i];
        uint8_t prev_pos = pos[cur];
        result[i] = prev_pos;
        for (int j = prev_pos - 1; j >= 0; --j) {
            swap(current_indexes[j], current_indexes[j + 1]);
            pos[current_indexes[j + 1]] = j + 1;
        }
        pos[cur] = 0;
    }

    return result;
}



struct Arithmetic {
    Model model = Model();
    bit_buffer buffer;
    int low = 0;
    int high = max_bits;
    int toSkip = 0;

    void writeBitAndSkip(int bit) {
        buffer.write_bit(bit);
        int negated = bit ^ 1;
        while (toSkip > 0) {
            buffer.write_bit(negated);
            toSkip--;
        }
    }

    void doJob(int index) {
        int range = high - low + 1;
        int total = model.cum[0];
        int chLow = model.cum[index];
        int chHigh = model.cum[index - 1];
        high = low + range * chHigh / total - 1;
        low = low + range * chLow / total;
        while (true) {

            if (high < q2) {
                writeBitAndSkip(0);
            } else if (low >= q2) {
                writeBitAndSkip(1);
                low -= q2;
                high -= q2;
            } else if (low >= q1 && high < q3) {
                toSkip++;
                low -= q1;
                high -= q1;
            } else {
                break;
            }
            low = 2 * low;
            high = 2 * high +1;
        }
    }

    bit_buffer encode(const vector<uint8_t> &input) {
        for (uint8_t ch: input) {
            int index = model.chToI[ch];
            doJob(index);
            model.updateModel(index);
        }
        doJob(numOfChars + 1);
        if (low < q1) {
            writeBitAndSkip(0);
        } else {
            writeBitAndSkip(1);
        }
        return buffer;
    }
};


vector<uint8_t> rle(const vector<uint8_t> &input) {
    vector<uint8_t> result;
    uint8_t prev = input[0];
    size_t prev_i = 0;
    size_t i = 1;
    while (prev_i < input.size()) {
//        if (rand() % 10 == 0) {
//        }
//        cout << prev_i << " " << i <<  " " << input.size() << endl;
        while (i < input.size() && input[i] != prev) {
            prev = input[i];
            i++;

        }
        int not_eq_len = i - prev_i - 1 + (i >= input.size() ? 1 : 0);
        if (not_eq_len > 0) {
            while (not_eq_len > 0) {
                int to_write = not_eq_len % 128;
                if (to_write == 0) {
                    to_write = 128;
                }
                result.push_back(-to_write);
                for (int j = prev_i; j < prev_i + to_write; ++j) {
                    result.push_back(input[j]);
//                    cout << not_eq_len << " " << prev_i << " " << i <<  " " << input.size() << endl;
                }
                prev_i += to_write;
                not_eq_len -= to_write;
            }
        } else {
            while (i < input.size() && input[i] == prev) {
                prev = input[i];
                i++;

            }
            int eq_len = i - prev_i;
            while (eq_len > 0) {
                int to_write = eq_len % 128;
                result.push_back(to_write);
                result.push_back(prev);
                prev_i += to_write == 0 ? 128 : to_write;
                eq_len -= to_write == 0 ? 128 : to_write;
//                cout << prev_i << " " << i <<  " " << input.size() << endl;
            }
            prev_i = i;
            prev = input[prev_i];
            i++;
        }

    }
    return result;
}

vector<uint8_t> rle2(const vector<uint8_t> &input) {
    vector<uint8_t> result;
    uint8_t prev = -1;
    int len = 0;
    for (uint8_t item : input) {
        if (item == prev) {
            len++;
        } else {
            if (len >= 2) {
                result.push_back(prev);
                result.push_back(len - 2);
            }
            result.push_back(item);
            len = 1;
        }
        if (len >= 257) {
            if (len >= 2) {
                result.push_back(prev);
                result.push_back(len - 2);
            }
            len = 1;
        }
        prev = item;
    }
    if (len >= 2) {
        result.push_back(prev);
        result.push_back(len - 2);
    }
    return result;
}


void build_code_rec(Node *root, unordered_map<uint8_t, code> &mapp, code &code) {
    if (root->id == nullptr) {
        code.add_bit(0);
        build_code_rec(root->left, mapp, code);
        code.remove_bit();
        code.add_bit(1);
        build_code_rec(root->right, mapp, code);
        code.remove_bit();
    } else {
        mapp[*(root->id)] = code;
    }
}

unordered_map<uint8_t, code> build_code(Node *root) {
    unordered_map<uint8_t, code> res;

    code c;

    c.add_bit(0);
    build_code_rec(root->left, res, c);
    c.remove_bit();
    c.add_bit(1);
    build_code_rec(root->right, res, c);
    c.remove_bit();
    return res;
}


pair<pair<bit_buffer, Node *>, size_t> huffman(const vector<uint8_t> &data) {
    unordered_map<uint8_t, size_t> freq;
    for (size_t i = 0; i < data.size(); ++i) {
        freq[data[i]]++;
    }
    multimap<size_t, Node *> nodes;
    for (const auto &pair: freq) {
        nodes.insert({pair.second, new Node(new uint8_t(pair.first), pair.second, nullptr, nullptr)});
    }
    while (nodes.size() > 1) {
        pair <
        const unsigned long, Node *> pair1 = nodes.begin().operator*();
        nodes.erase(nodes.begin());
        pair <
        const unsigned long, Node *> pair2 = nodes.begin().operator*();
        nodes.erase(nodes.begin());
        Node *new_node = new Node(nullptr, pair1.first + pair2.first, pair1.second, pair2.second);
        nodes.insert({new_node->sum, new_node});
    }

    Node *root = nodes.begin()->second;
//    print_tree(root);
    unordered_map<uint8_t, code> coded = build_code(root);

    bit_buffer res;
    for (const auto &item: data) {
        res.add_bits(coded[item]);
    }
    return {{res, root}, freq.size()};
}

void out_tree_rec(ofstream &out, Node *root, vector<uint8_t> &syms) {
    if (root->id == nullptr) {
        char one = 1;
        out.write(&one, 1);
        out_tree_rec(out, root->left, syms);
        out_tree_rec(out, root->right, syms);
    } else {
        char zero = 0;
        out.write(&zero, 1);
        syms.push_back(*root->id);
    }
}

void out_tree(ofstream &out, Node *root) {
    vector<uint8_t> syms;
    out_tree_rec(out, root, syms);
    out.write((char *) syms.data(), syms.size());
}

vector<uint8_t> decode_rle(const vector<uint8_t> &input) {
    vector<uint8_t> result;
    int i = 0;
    while (i < input.size()) {
        int val = input[i];
        if (val < 128) {
            if (val == 0) {
                val = 128;
            }
            for (int j = 0; j < val; ++j) {
                result.push_back(input[i + 1]);
            }
            i += 2;
        } else {
            val = -int((char) input[i]);
            for (int j = 0; j < val; ++j) {
                result.push_back(input[i + 1 + j]);
            }
            i += val + 1;
        }
    }
    return result;
}

int main(int argc, char *argv[]) {
    //coding

//    vector<uint8_t> a = convertToUintVector("wwzessxddrctvfybguhnimwzexscdrvftbgyhunwzesxdrctvfybguhnimwzexscdrvftbgyhunwzesxdrctvfybguhnimwzexscdrvftbgyhunAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabc");
//    vector<uint8_t> c = rle2(a);
//    vector<uint8_t> d = decode_rle2(c);
    if (argc < 2) {
        cout << "coder.exe input_file <output_file>" << endl;
        return 0;
    }


    std::ifstream in(argv[1], std::ios_base::binary);
    vector<uint8_t> input((std::istreambuf_iterator<char>(in)),
                          std::istreambuf_iterator<char>());
    in.close();

    vector<uint8_t> alph;
    for (int i = 0; i < 256; ++i) {
        alph.push_back(i);
    }

    const pair<const vector<uint8_t>, size_t> bwt_result = bwt(input);
    const vector<uint8_t> mft_result = mft(bwt_result.first, alph);
    const vector<uint8_t> rle_result = rle2(mft_result);
    bit_buffer b  = Arithmetic().encode(rle_result);

    string outf(argv[1]);

    if (argc >= 3) {
        outf = string(argv[2]);
    } else {
        outf.append(".coded");
    }

    ofstream out(outf, std::ios_base::binary);

    out.write((char *) &(b.pos), sizeof(b.pos));
    out.write((char *) (b.data.data()), sizeof(uint8_t) * b.data.size());

    out.write((char *) &(bwt_result.second), sizeof(bwt_result.second));


    out.close();
    return 0;
}
