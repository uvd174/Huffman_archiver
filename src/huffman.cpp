#include "huffman.h"

#include <algorithm>
#include <iostream>
#include <cstring>

namespace huff {

//==================================TreeNode=================================//

TreeNode::TreeNode()
    : used_(false), type_(EMPTY), symbol_(0), amount_(0),
      left_(nullptr), right_(nullptr) {}

TreeNode::TreeNode(char symbol, uint32_t amount)
    : used_(false), type_(EXTERNAL), symbol_(symbol), amount_(amount),
      left_(nullptr), right_(nullptr) {}

TreeNode::TreeNode(std::pair<const char, uint32_t> sym_am)
    : used_(false), type_(EXTERNAL), symbol_(sym_am.first),
      amount_(sym_am.second), left_(nullptr), right_(nullptr) {}

TreeNode::TreeNode(TreeNode *left, TreeNode *right)
    : used_(false), type_(INTERNAL), symbol_(0),
      amount_(left->amount() + right->amount()),
      left_(left), right_(right) {
  left->used(true);
  right->used(true);
}

bool TreeNode::operator==(const TreeNode &other) const {
  return used()   == other.used()   &&
         type()   == other.type()   &&
         symbol() == other.symbol() &&
         amount() == other.amount() &&
         left()   == other.left()   &&
         right()  == other.right();
}

bool TreeNode::operator<(const TreeNode &other) const {
  if (other.used()) {
    return true;
  }
  if (used()) {
    return false;
  }
  return std::make_pair(amount(), symbol()) <
         std::make_pair(other.amount(), other.symbol());
}

void TreeNode::used(bool used_flag) {
  used_ = used_flag;
}

bool TreeNode::used() const {
  return used_;
}

TreeNode::Type TreeNode::type() const {
  return type_;
}

char TreeNode::symbol() const {
  return symbol_;
}

uint32_t TreeNode::amount() const {
  return amount_;
}

TreeNode *TreeNode::left() {
  return left_;
}

const TreeNode *TreeNode::left() const {
  return left_;
}

TreeNode *TreeNode::right() {
  return right_;
}

const TreeNode *TreeNode::right() const {
  return right_;
}

//==================================TreeNode=================================//

//=================================BitWriter=================================//

BitWriter::BitWriter(std::ostream &out) : buffer_(0), size_(0), out_(out) {}

void BitWriter::write(const BitBuffer &bit_buffer) {
  int loop_count = bit_buffer.size / 8;
  for (int i = 0; i < loop_count; ++i) {
    buffer_ |= static_cast<uint16_t>(bit_buffer.buffer[i]) << size_;
    out_.write(reinterpret_cast<char *>(&buffer_), sizeof(char));
    buffer_ >>= 8;
  }
  int residue = bit_buffer.size % 8;
  if (residue) {
    buffer_ |= static_cast<uint16_t>(bit_buffer.buffer[loop_count] &
                                     ((1UL << residue) - 1)) << size_;
    size_ += residue;
    if (size_ > 7) {
      out_.write(reinterpret_cast<char *>(&buffer_), sizeof(char));
      buffer_ >>= 8;
      size_ -= 8;
    }
  }
}

void BitWriter::flush() {
  if (size_ == 0) {
    return;
  }
  out_.write(reinterpret_cast<char *>(&buffer_), sizeof(char));
  buffer_ >>= 8;
  size_ = 0;
}

//=================================BitWriter=================================//

//==================================HuffTree=================================//

BitBuffer::BitBuffer(uint16_t size) : size(size), buffer() {
  memset(buffer, 0, sizeof(uint8_t) * 32);
}

BitBuffer &BitBuffer::operator=(const BitBuffer &other) {
  if (this == &other) {
    return *this;
  }
  size = other.size;
  memcpy(buffer, other.buffer, sizeof(uint8_t) * 32);
  return *this;
}

HuffTree::HuffTree(std::map<char, uint32_t> &amount_table) {
  build_tree(amount_table);
  try {
    extract_codes();
  } catch (const std::logic_error &e) {
    return;
  }
}

HuffTree::HuffTree(const HuffTree &other) {
  *this = other;
}

HuffTree &HuffTree::operator=(const HuffTree &other) {
  if (this == &other) {
    return *this;
  }
  std::map<char, uint32_t> amount_table;
  for (auto &elem : other.tree_) {
    if (elem.type() == TreeNode::EXTERNAL) {
      amount_table[elem.symbol()] = elem.amount();
    }
  }
  build_tree(amount_table);
  char_buffer_map_ = other.char_buffer_map_;
  return *this;
}

const TreeNode *HuffTree::root() const {
  emptiness_check();
  return &tree_.back();
}

uint8_t HuffTree::leaves_count() const {
  emptiness_check();
  return (tree_.size() - 1) / 2;
}

void HuffTree::build_tree(std::map<char, uint32_t> &amount_table) {
  tree_.clear();
  tree_.reserve(2 * amount_table.size());
  for (auto &elem : amount_table) {
    tree_.emplace_back(elem);
  }
  int loop_count = static_cast<int>(tree_.size()) - 1;
  for (int i = 0; i < loop_count; ++i) {
    TreeNode *first_min = &*std::min_element(tree_.begin(), tree_.end());
    first_min->used(true);
    TreeNode *second_min = &*std::min_element(tree_.begin(), tree_.end());
    tree_.emplace_back(first_min, second_min);
  }
}

void HuffTree::save_tree_info(std::ostream &out) const {
  emptiness_check();
  uint8_t leaves = leaves_count();
  out.write(reinterpret_cast<char *>(&leaves), sizeof leaves);
  uint32_t bit_sum = 0;
  for (auto &node : tree_) {
    if (node.type() == TreeNode::EXTERNAL) {
      char symbol = node.symbol();
      uint32_t amount = node.amount();
      bit_sum += amount * char_buffer_map_.at(symbol).size;
      out.write(&symbol, sizeof symbol);
      out.write(reinterpret_cast<char *>(&amount), sizeof amount);
    }
  }
  bit_sum %= 8;
  if (tree_.size() != 1) {
    out.write(reinterpret_cast<char *>(&bit_sum), sizeof(char));
  }
}

void HuffTree::extract_codes() {
  extract_codes(char_buffer_map_);
}

void
HuffTree::extract_codes(std::map<char, BitBuffer> &char_buffer_map) const {
  emptiness_check();
  huff::BitBuffer bit_buffer;
  extract_codes_rec(char_buffer_map, root(), bit_buffer);
}

void HuffTree::extract_codes_rec(std::map<char, BitBuffer> &char_buffer_map,
                                 const TreeNode *node,
                                 BitBuffer &bit_buffer) const {
  if (node->type() == huff::TreeNode::EXTERNAL) {
    char_buffer_map[node->symbol()] = bit_buffer;
    --bit_buffer.size;
    return;
  }

  if (node->left()) {
    int byte = bit_buffer.size / 8;
    int offset = bit_buffer.size % 8;
    ++bit_buffer.size;
    extract_codes_rec(char_buffer_map, node->left(), bit_buffer);
    bit_buffer.buffer[byte] |= 1UL << offset;
    ++bit_buffer.size;
    extract_codes_rec(char_buffer_map, node->right(), bit_buffer);
    bit_buffer.buffer[byte] &= ~(1UL << offset);
  }
  --bit_buffer.size;
}

BitBuffer &HuffTree::operator[](char symbol) {
  return char_buffer_map_.at(symbol);
}

const BitBuffer &HuffTree::operator[](char symbol) const {
  return char_buffer_map_.at(symbol);
}

void HuffTree::emptiness_check() const {
  if (tree_.empty()) {
    throw std::logic_error("The tree is empty!");
  }
}

//==================================HuffTree=================================//

//==============================HuffmanArchiver==============================//

long HuffmanArchiver::encode(std::istream &in,
                             std::ostream &out) {
  encode_buildHuffTree(in);

  try {
    tree().extract_codes();
  } catch (const std::logic_error &e) {
    return 0;
  }

  tree().save_tree_info(out);
  long tree_info_size = out.tellp();

  BitWriter bit_writer(out);
  char symbol;
  while (in.read(&symbol, sizeof symbol)) {
    bit_writer.write(tree()[symbol]);
  }
  bit_writer.flush();

  in.clear();

  return tree_info_size;
}

long HuffmanArchiver::decode(std::istream &in,
                             std::ostream &out) {
  decode_buildHuffTree(in);

  try {
    if (tree().root()->type() == TreeNode::EXTERNAL) {
      char symbol = tree().root()->symbol();
      for (size_t i = 0; i < tree().root()->amount(); ++i) {
        out.write(&symbol, sizeof symbol);
      }
      return in.tellg();
    }
  } catch (const std::logic_error &e) {
    in.clear();
    return 0;
  }

  uint8_t last_byte_size;
  in.read(reinterpret_cast<char *>(&last_byte_size), sizeof last_byte_size);
  check_format(in);

  long tree_info_size = in.tellg();
  in.seekg(0, std::ios_base::end);
  long file_length = in.tellg() - tree_info_size;
  in.seekg(tree_info_size);

  const TreeNode *cur_node = tree().root();
  uint8_t cur_byte = 0;
  for (int i = 0; i < file_length - 1 + (last_byte_size == 0); ++i) {
    in.read(reinterpret_cast<char *>(&cur_byte), sizeof cur_byte);
    cur_node = process_byte(cur_node, cur_byte, 8, out);
  }

  if (last_byte_size > 0) {
    in.read(reinterpret_cast<char *>(&cur_byte), sizeof cur_byte);
    process_byte(cur_node, cur_byte, last_byte_size, out);
  }

  return tree_info_size;
}

const TreeNode *HuffmanArchiver::process_byte(const TreeNode *cur_node,
                                              uint8_t byte, int size,
                                              std::ostream &out) {
  for (int i = 0; i < size; ++i) {
    uint8_t cur_bit = byte & (1U << i);

    if (cur_bit) {
      cur_node = cur_node->right();
    } else {
      cur_node = cur_node->left();
    }

    if (cur_node->type() == TreeNode::EXTERNAL) {
      char symbol = cur_node->symbol();
      out.write(&symbol, sizeof symbol);
      cur_node = tree().root();
    }
  }
  return cur_node;
}

void HuffmanArchiver::encode_buildHuffTree(std::istream &in) {
  std::map<char, uint32_t> amount_table;
  char symbol;
  while (in.read(&symbol, sizeof symbol)) {
    ++amount_table[symbol];
  }
  huff_tree_.build_tree(amount_table);
  in.clear();
  in.seekg(0);
}

void HuffmanArchiver::decode_buildHuffTree(std::istream &in) {
  std::map<char, uint32_t> amount_table;

  uint8_t size;
  uint32_t amount;
  char symbol;

  in.read(reinterpret_cast<char *>(&size), sizeof size);
  if (in.fail()) {
    huff_tree_.build_tree(amount_table);
    return;
  }
  for (int i = 0; i <= size; ++i) {
    in.read(&symbol, sizeof symbol);
    in.read(reinterpret_cast<char *>(&amount), sizeof amount);
    amount_table[symbol] = amount;
  }
  check_format(in);
  huff_tree_.build_tree(amount_table);
}

HuffTree &HuffmanArchiver::tree() {
  return huff_tree_;
}

void HuffmanArchiver::check_format(std::istream &in) {
  if (in.fail()) {
    throw std::runtime_error("File format error!");
  }
}

//==============================HuffmanArchiver==============================//

}