#include <iostream>
#include <utility>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include "lib.h"

using namespace std;

vector<uint8_t> from_mft(const vector<uint8_t> &input, const vector<uint8_t> &sorted_alph) {
    vector<uint8_t> current_indexes(sorted_alph.size());
    unordered_map<uint8_t, size_t> pos;

    for (size_t i = 0; i < sorted_alph.size(); ++i) {
        current_indexes[i] = sorted_alph[i];
        pos[i] = i;
    }
    vector<uint8_t> result(input.size());

    for (size_t i = 0; i < input.size(); ++i) {
        uint8_t cur = input[i];
        result[i] = current_indexes[cur];
        for (int j = cur - 1; j >= 0; --j) {
            swap(current_indexes[j], current_indexes[j + 1]);
        }
    }

    return result;
}


vector<uint8_t> from_bwt(const vector<uint8_t> &input, size_t initial_pos, const vector<uint8_t> &sorted_alph) {
    vector<uint8_t> result(input.size());

    unordered_map<uint8_t, size_t> count_prefix;
    vector<size_t> freq(input.size());

    for (size_t i = 0; i < input.size(); ++i) {
        freq[i] = count_prefix[input[i]];
        count_prefix[input[i]]++;
    }

    unordered_map<uint8_t, size_t> count_less;
    for (size_t i = 1; i < sorted_alph.size(); ++i) {
        count_less[sorted_alph[i]] = count_less[sorted_alph[i - 1]] + count_prefix[sorted_alph[i - 1]];
    }
    result[input.size() - 1] = input[initial_pos];

    for (signed long i = input.size() - 2; i >= 0; --i) {
        initial_pos = freq[initial_pos] + count_less[input[initial_pos]];
        result[i] = input[initial_pos];
    }

    return result;
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

vector<uint8_t> decode_rle2(const vector<uint8_t> &input) {
    vector<uint8_t> result;
    int prev = -1;
    int i = 0;
    while (i < input.size()) {
        int item = input[i];
        result.push_back(item);
        if (item == prev) {
            int len = input[++i];
            for (int j = 0; j < len; ++j) {
                result.push_back(prev);
            }
        }
        prev = item;
        i++;
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

pair<Node *, vector<uint8_t>> read_tree(char *data, size_t alph_size) {
    vector<uint8_t> syms;
    vector<Node *> nodes;
    vector<Node *> stack;

    for (int i = 0; i < alph_size * 2 - 1; ++i) {
        if (data[i] == 0) {
            Node *child = new Node(nullptr, 0, nullptr, nullptr);
            Node *parent = stack[stack.size() - 1];
            parent->add_child(child);
            while (stack[stack.size() - 1]->has_two_kids()) {
                if (stack.size() == 1) {
                    break;
                }
                Node *kid = stack[stack.size() - 1];
                stack.pop_back();

                Node *par = stack[stack.size() - 1];
                par->add_child(kid);
            }
            nodes.push_back(child);
        } else if (data[i] == 1) {
            Node *new_node = new Node(nullptr, 0, nullptr, nullptr);
            stack.push_back(new_node);
        } else {
            throw runtime_error("asd");
        }
    }
    data = data + alph_size * 2 - 1;
    vector<uint8_t> alph;
    for (int i = 0; i < alph_size; ++i) {
        nodes[i]->id = new uint8_t(data[i]);
        alph.push_back(data[i]);
    }
    sort(alph.begin(), alph.end());
    return {stack[0], alph};
}

vector<uint8_t> from_huffman(bit_buffer &buffer, Node *root) {
    vector<uint8_t> res;

    Node *cur = root;

    for (int i = 0; i < buffer.pos; ++i) {
        if (cur->id != nullptr) {
            res.push_back(*(cur->id));
            cur = root;
        }
        int bit = buffer.read_bit();
        if (bit == 0) {
            cur = cur->left;
        } else {
            cur = cur->right;
        }

    }
    if (cur->id != nullptr) {
        res.push_back(*(cur->id));
    }

    return res;
}

struct ArithmeticDecoder {
    Model model = Model();
    bit_buffer buffer;
    int low = 0;
    int high = max_bits;
    int value = 0;

    void moveForward() {
        int bit = buffer.read_bit();
        value = 2 * value + bit;
    }

    explicit ArithmeticDecoder(const bit_buffer &buffer) : buffer(buffer) {
        for (int i = 0; i < bits; ++i) {
            moveForward();
        }
    }

    int parseSymbol(int cum) {
        int tmp = 1;
        while (model.cum[tmp] > cum) {
            tmp++;
        }
        return tmp;
    }

    int nextSymbolIndex() {
        int range = high - low + 1;
        int sum = model.cum[0];
        int cum = ((value - low + 1) * sum - 1) / range;
        int index = parseSymbol(cum);
        int symLow = model.cum[index];
        int symHigh = model.cum[index - 1];
        high = low + range * symHigh / sum - 1;
        low = low + range * symLow / sum;
        while (true) {
            if (high < q2) {

            } else if (low >= q2) {
                value -= q2;
                low -= q2;
                high -= q2;
            } else if (low >= q1 && high < q3) {
                value -= q1;
                low -= q1;
                high -= q1;
            } else {
                break;
            }
            low = 2 * low;
            high = high * 2 + 1;
            moveForward();
        }
        return index;
    }

    vector<uint8_t> decode() {
        vector<uint8_t> result;
        while (true) {
            int index = nextSymbolIndex();
            if (index == 257) {
                break;
            }
            int res = model.iToCh[index];
            result.push_back(res);
            model.updateModel(index);
        }
        return result;
    }
};

int main(int argc, char *argv[]) {
    //coding
    if (argc < 2) {
        cout << "decoder.exe input_file <output_file>" << endl;
        return 0;
    }

    //decoding

    std::ifstream in2(argv[1], std::ios_base::binary);

    std::string input_file((std::istreambuf_iterator<char>(in2)),
                           std::istreambuf_iterator<char>());
    in2.close();

    char *data = const_cast<char *>(input_file.data());
    size_t shift = 0;

    size_t pos = 0;
    memcpy(&pos, data, sizeof(size_t));
    shift += sizeof(size_t);

    size_t len = (pos + 7) / 8;
    vector<uint8_t> buffer(len);
    for (int i = 0; i < len; ++i) {
        buffer[i] = data[shift + i];
    }
    shift += len;

    bit_buffer bitBuffer;
    bitBuffer.pos = pos;
    bitBuffer.data = buffer;

//    size_t alph_size = 0;
//    memcpy(&alph_size, data + shift, sizeof(size_t));
//    shift += sizeof(size_t);

//    size_t init_alpha_size = 0;
//    memcpy(&init_alpha_size, shift + data, sizeof(size_t));
//    shift += sizeof(size_t);
//
//    pair<Node *, vector<uint8_t>> tree = read_tree(data + shift, alph_size);
//    Node *root = tree.first;
//    vector<uint8_t> sorted_alph(init_alpha_size);
//
//    shift += alph_size * 3 - 1;


    size_t initial_pos = 0;
    memcpy(&initial_pos, shift + data, sizeof(size_t));
    shift += sizeof(size_t);

//    memcpy(sorted_alph.data(), shift + data, init_alpha_size);


//    const vector<uint8_t> huffman_decoded = from_huffman(bitBuffer, root);
    const vector<uint8_t> arithmetic_decoded = ArithmeticDecoder(bitBuffer).decode();
//    cout << convertNumbersToString(huffman_decoded) << endl;

    vector<uint8_t> alph;
    for (int i = 0; i < 256; ++i) {
        alph.push_back(i);
    }

    const vector<uint8_t> decoded_rle = decode_rle2(arithmetic_decoded);

    const vector<uint8_t> &from_mft_result = from_mft(decoded_rle, alph);
//    cout << convertToString(from_mft_result) << ", " << initial_pos << endl;

    const vector<uint8_t> &from_bwt_result = from_bwt(from_mft_result, initial_pos, alph);

    string outf(argv[1]);

    if (argc >= 3) {
        outf = string(argv[2]);
    } else {
        outf.append(".decoded");
    }
//    print_tree(root);


    ofstream out(outf, std::ios_base::binary);
    out.write((char *) from_bwt_result.data(), from_mft_result.size());

    out.close();

    return 0;
}