#ifndef HW_02_HUFFMAN_H
#define HW_02_HUFFMAN_H

#include <set>
#include <map>
#include <string>

namespace huff {

class TreeNode {
 public:
  enum Type {
    INTERNAL, EXTERNAL, EMPTY
  };

  TreeNode();
  TreeNode(char symbol, uint32_t amount);
  explicit TreeNode(std::pair<const char, uint32_t> sym_am);
  TreeNode(TreeNode *left, TreeNode *right);
  TreeNode(const TreeNode &other) = default;
  TreeNode &operator=(const TreeNode &other) = default;
  ~TreeNode() = default;

  bool operator==(const TreeNode &other) const;
  bool operator<(const TreeNode &node2) const;

  void used(bool used_flag);
  bool used() const;

  Type type() const;
  char symbol() const;
  uint32_t amount() const;

  TreeNode *left();
  const TreeNode *left() const;

  TreeNode *right();
  const TreeNode *right() const;

 private:
  bool used_;
  Type type_;
  char symbol_;
  uint32_t amount_;
  TreeNode *left_;
  TreeNode *right_;
};

struct BitBuffer {
  explicit BitBuffer(uint16_t size = 0);
  BitBuffer &operator=(const BitBuffer &other);

  uint16_t size;
  uint8_t buffer[32];
};

class BitWriter {
 public:
  explicit BitWriter(std::ostream &out);

  void write(const BitBuffer &bit_buffer);
  void flush();

 private:
  uint16_t buffer_;
  uint8_t size_;
  std::ostream &out_;
};

class HuffTree {
 public:
  HuffTree() = default;
  explicit HuffTree(std::map<char, uint32_t> &amount_table);
  HuffTree(const HuffTree &other);
  HuffTree &operator=(const HuffTree &other);
  ~HuffTree();

  const TreeNode *root() const;
  uint8_t leaves_count() const;

  void build_tree(std::map<char, uint32_t> &amount_table);
  void save_tree_info(std::ostream &out) const;

  void extract_codes();
  void extract_codes(std::map<char, BitBuffer> &char_buffer_map) const;

  BitBuffer &operator[](char symbol);
  const BitBuffer &operator[](char symbol) const;

 private:
  void extract_codes_rec(std::map<char, BitBuffer> &char_buffer_map,
                         const TreeNode *node, BitBuffer &bit_buffer) const;
  void emptiness_check() const;

  std::set<TreeNode> tree_;
  std::map<char, BitBuffer> char_buffer_map_;
};

class HuffmanArchiver {
 public:
  long encode(std::istream &in, std::ostream &out);
  long decode(std::istream &in, std::ostream &out);

  void encode_buildHuffTree(std::istream &in);
  void decode_buildHuffTree(std::istream &in);

  HuffTree &tree();

 private:
  const TreeNode *process_byte(const TreeNode *cur_node, uint8_t byte,
                               int size, std::ostream &out);
  static void check_format(std::istream &in);

  HuffTree huff_tree_;
};

} //namespace huff

#endif //HW_02_HUFFMAN_H
