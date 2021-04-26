#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest.h"
#include "huffman.h"


TEST_CASE("testing the TreeNode class") {

  SUBCASE("testing constructing a node from a symbol and it's amount") {
    huff::TreeNode node('a', 100);
    CHECK_EQ(node.used(), false);
    CHECK_EQ(node.symbol(), 'a');
    CHECK_EQ(node.type(), huff::TreeNode::EXTERNAL);
    CHECK_EQ(node.amount(), 100);
    CHECK_EQ(node.left(), nullptr);
    CHECK_EQ(node.right(), nullptr);
  }

  SUBCASE("testing constructing a node from a <symbol, amount> pair") {
    huff::TreeNode node1(std::make_pair('a', 100));
    huff::TreeNode node2('a', 100);
    CHECK_EQ(node1, node2);
  }

  SUBCASE("testing default constructor") {
    huff::TreeNode node;
    CHECK_EQ(node.used(), false);
    CHECK_EQ(node.symbol(), 0);
    CHECK_EQ(node.type(), huff::TreeNode::EMPTY);
    CHECK_EQ(node.amount(), 0);
    CHECK_EQ(node.left(), nullptr);
    CHECK_EQ(node.right(), nullptr);
  }

  SUBCASE("testing parent constructor") {
    huff::TreeNode left('a', 100);
    huff::TreeNode right('b', 150);
    huff::TreeNode parent_node(&left, &right);
    CHECK_EQ(parent_node.used(), false);
    CHECK_EQ(parent_node.symbol(), 0);
    CHECK_EQ(parent_node.type(), huff::TreeNode::INTERNAL);
    CHECK_EQ(parent_node.amount(), 250);
    CHECK_EQ(parent_node.left(), &left);
    CHECK_EQ(parent_node.right(), &right);
    CHECK_EQ(left.used(), true);
    CHECK_EQ(right.used(), true);
  }

  SUBCASE("testing huff::TreeNode::operator==") {
    CHECK_EQ(huff::TreeNode('a', 100), huff::TreeNode('a', 100));
    CHECK_FALSE(huff::TreeNode('a', 100) == huff::TreeNode('a', 200));
    CHECK_FALSE(huff::TreeNode('a', 100) == huff::TreeNode('b', 100));
  }

  SUBCASE("testing TreeNode-container compatibility") {
    std::vector<huff::TreeNode> vec(3);

    SUBCASE("testing huff::TreeNode::operator=") {
      vec[0] = huff::TreeNode('c', 200);
      vec[1] = huff::TreeNode('a', 100);
      vec[2] = huff::TreeNode('b', 100);
      CHECK_EQ(vec[0], huff::TreeNode('c', 200));
      CHECK_EQ(vec[1], huff::TreeNode('a', 100));
      CHECK_EQ(vec[2], huff::TreeNode('b', 100));

      SUBCASE("testing huff::TreeNode::operator<") {
        CHECK_LT(huff::TreeNode('a', 100), huff::TreeNode('b', 200));
        CHECK_LT(huff::TreeNode('c', 100), huff::TreeNode('d', 100));
        CHECK_FALSE(huff::TreeNode('f', 100) < huff::TreeNode('e', 100));
        CHECK_FALSE(huff::TreeNode('e', 200) < huff::TreeNode('f', 100));

        SUBCASE("testing vector<TreeNode> sorting") {
          sort(vec.begin(), vec.end());
          CHECK_EQ(vec[0], huff::TreeNode('a', 100));
          CHECK_EQ(vec[1], huff::TreeNode('b', 100));
          CHECK_EQ(vec[2], huff::TreeNode('c', 200));
        }
      }
    }
  }
}


TEST_CASE("testing BitBuffer::operator=") {
  huff::BitBuffer bit_buffer(5);
  CHECK_EQ(bit_buffer.size, 5);
  bit_buffer.buffer[0] = 0b00010101;
  huff::BitBuffer bit_buffer_copy;
  bit_buffer_copy = bit_buffer;
  CHECK_EQ(bit_buffer_copy.buffer[0], 0b00010101);
  CHECK_EQ(bit_buffer_copy.size, 5);
}


TEST_CASE("testing BitWriter class") {
  std::ostringstream out(std::ios::binary);
  huff::BitBuffer bit_buffer;
  huff::BitWriter bit_writer(out);
  bit_buffer.size = 12;
  bit_buffer.buffer[0] = static_cast<uint8_t>('a');
  bit_buffer.buffer[1] = static_cast<uint8_t>('\x62');
  bit_writer.write(bit_buffer);
  CHECK_EQ(out.str(), "a");
  bit_buffer.buffer[0] = static_cast<uint8_t>('\x36');
  bit_buffer.buffer[1] = static_cast<uint8_t>('\xF6');
  bit_writer.write(bit_buffer);
  CHECK_EQ(out.str(), "abc");
  bit_buffer.size = 8;
  bit_buffer.buffer[0] = static_cast<uint8_t>('d');
  bit_writer.write(bit_buffer);
  CHECK_EQ(out.str(), "abcd");
}


TEST_CASE("testing the HuffTree class") {
  SUBCASE("testing constructing a tree from an amount table") {
    std::map<char, uint32_t> amount_table = { {'a', 1}, {'b', 2}, {'c', 4} };
    huff::HuffTree huff_tree(amount_table);
    REQUIRE_NOTHROW(huff_tree.root());
    REQUIRE_FALSE(huff_tree.root()->left() == nullptr);
    REQUIRE_FALSE(huff_tree.root()->left()->left() == nullptr);
    REQUIRE_FALSE(huff_tree.root()->left()->right() == nullptr);
    REQUIRE_FALSE(huff_tree.root()->right() == nullptr);
    huff::TreeNode a_node('a', 1);
    huff::TreeNode b_node('b', 2);
    huff::TreeNode c_node('c', 4);
    a_node.used(true);
    b_node.used(true);
    c_node.used(true);
    CHECK_EQ(*huff_tree.root()->left()->left(), a_node);
    CHECK_EQ(*huff_tree.root()->left()->right(), b_node);
    CHECK_EQ(*huff_tree.root()->right(), c_node);
    CHECK_EQ(huff_tree.root()->amount(), 7);
  }

  SUBCASE("testing constructing a tree from an empty amount table") {
    std::map<char, uint32_t> amount_table = {};
    huff::HuffTree huff_tree(amount_table);
    CHECK_THROWS_WITH_AS(huff_tree.root(),
                         "The tree is empty!", std::logic_error);
  }

  SUBCASE("testing constructing a tree from an amount table with one entry") {
    std::map<char, uint32_t> amount_table = { {'a', 100} };
    huff::HuffTree huff_tree(amount_table);
    CHECK_EQ(*huff_tree.root(), huff::TreeNode('a', 100));
  }

  SUBCASE("testing HuffTree::operator=") {
    std::map<char, uint32_t> amount_table = { {'a', 1}, {'b', 2}, {'c', 4} };
    huff::HuffTree huff_tree_copy;
    {
      huff::HuffTree huff_tree(amount_table);
      huff_tree_copy = huff_tree;
    }
    REQUIRE_NOTHROW(huff_tree_copy.root());
    REQUIRE_FALSE(huff_tree_copy.root()->left() == nullptr);
    REQUIRE_FALSE(huff_tree_copy.root()->left()->left() == nullptr);
    REQUIRE_FALSE(huff_tree_copy.root()->left()->right() == nullptr);
    REQUIRE_FALSE(huff_tree_copy.root()->right() == nullptr);
    huff::TreeNode a_node('a', 1);
    huff::TreeNode b_node('b', 2);
    huff::TreeNode c_node('c', 4);
    a_node.used(true);
    b_node.used(true);
    c_node.used(true);
    CHECK_EQ(*huff_tree_copy.root()->left()->left(), a_node);
    CHECK_EQ(*huff_tree_copy.root()->left()->right(), b_node);
    CHECK_EQ(*huff_tree_copy.root()->right(), c_node);
    CHECK_EQ(huff_tree_copy.root()->amount(), 7);
  }

  SUBCASE("testing HuffTree::leaves_count") {
    std::map<char, uint32_t> amount_table = { {'a', 1}, {'b', 2}, {'c', 4} };
    huff::HuffTree huff_tree(amount_table);
    CHECK_EQ(huff_tree.leaves_count(), 2);
    amount_table = { {'a', 2} };
    huff_tree.build_tree(amount_table);
    CHECK_EQ(huff_tree.leaves_count(), 0);
    amount_table = {};
    huff_tree.build_tree(amount_table);
    CHECK_THROWS_WITH_AS(huff_tree.leaves_count(),
                         "The tree is empty!", std::logic_error);
  }

  SUBCASE("testing HuffTree::save_tree_info method") {
    std::map<char, uint32_t> amount_table = { {'a', 1}, {'b', 2}, {'c', 4} };
    huff::HuffTree huff_tree(amount_table);
    REQUIRE_NOTHROW(huff_tree.extract_codes());
    std::ostringstream out(std::ios::binary);
    REQUIRE_NOTHROW(huff_tree.save_tree_info(out));
    std::string test_str{2, 'a', 1, 0, 0, 0,
                            'b', 2, 0, 0, 0,
                            'c', 4, 0, 0, 0, 2};
    CHECK_EQ(out.str(), test_str);
  }

  SUBCASE("testing HuffTree::extract_codes method") {
    std::map<char, uint32_t> amount_table = { {'a', 1}, {'b', 2}, {'c', 4} };
    huff::HuffTree huff_tree(amount_table);
    REQUIRE_NOTHROW(huff_tree.extract_codes());
    CHECK_EQ(huff_tree['a'].size, 2);
    CHECK_EQ(huff_tree['b'].size, 2);
    CHECK_EQ(huff_tree['c'].size, 1);
    CHECK_EQ(huff_tree['a'].buffer[0], 0b00000000);
    CHECK_EQ(huff_tree['b'].buffer[0], 0b00000010);
    CHECK_EQ(huff_tree['c'].buffer[0], 0b00000001);
  }
}


TEST_CASE("testing HuffmanArchiver class") {
  huff::HuffmanArchiver huffman_archiver;

  SUBCASE("testing encode_buildHuffTree method") {
    std::istringstream encode_str("cbcacbc", std::ios::binary);
    REQUIRE_NOTHROW(huffman_archiver.encode_buildHuffTree(encode_str));
    REQUIRE_NOTHROW(huffman_archiver.tree().root());
    REQUIRE_FALSE(huffman_archiver.tree().root()->left() == nullptr);
    REQUIRE_FALSE(huffman_archiver.tree().root()->left()->left() == nullptr);
    REQUIRE_FALSE(huffman_archiver.tree().root()->left()->right() == nullptr);
    REQUIRE_FALSE(huffman_archiver.tree().root()->right() == nullptr);
    huff::TreeNode a_node('a', 1);
    huff::TreeNode b_node('b', 2);
    huff::TreeNode c_node('c', 4);
    a_node.used(true);
    b_node.used(true);
    c_node.used(true);
    CHECK_EQ(*huffman_archiver.tree().root()->left()->left(), a_node);
    CHECK_EQ(*huffman_archiver.tree().root()->left()->right(), b_node);
    CHECK_EQ(*huffman_archiver.tree().root()->right(), c_node);
    CHECK_EQ(huffman_archiver.tree().root()->amount(), 7);
  }

  SUBCASE("testing decode_buildHuffTree method") {
    std::string test_str{2, 'a', 1, 0, 0, 0,
                            'b', 2, 0, 0, 0,
                            'c', 4, 0, 0, 0};
    std::istringstream decode_str(test_str, std::ios::binary);
    std::cout << std::endl;
    REQUIRE_NOTHROW(huffman_archiver.decode_buildHuffTree(decode_str));
    REQUIRE_NOTHROW(huffman_archiver.tree().root());
    REQUIRE_FALSE(huffman_archiver.tree().root()->left() == nullptr);
    REQUIRE_FALSE(huffman_archiver.tree().root()->left()->left() == nullptr);
    REQUIRE_FALSE(huffman_archiver.tree().root()->left()->right() == nullptr);
    REQUIRE_FALSE(huffman_archiver.tree().root()->right() == nullptr);
    huff::TreeNode a_node('a', 1);
    huff::TreeNode b_node('b', 2);
    huff::TreeNode c_node('c', 4);
    a_node.used(true);
    b_node.used(true);
    c_node.used(true);
    CHECK_EQ(*huffman_archiver.tree().root()->left()->left(), a_node);
    CHECK_EQ(*huffman_archiver.tree().root()->left()->right(), b_node);
    CHECK_EQ(*huffman_archiver.tree().root()->right(), c_node);
    CHECK_EQ(huffman_archiver.tree().root()->amount(), 7);
  }

  SUBCASE("testing decode_buildHuffTree method on an empty file") {
    std::string test_str = {};
    std::istringstream decode_str(test_str, std::ios::binary);
    REQUIRE_NOTHROW(huffman_archiver.decode_buildHuffTree(decode_str));
    CHECK_THROWS_WITH_AS(huffman_archiver.tree().root(),
                         "The tree is empty!", std::logic_error);
  }

  SUBCASE("testing decode_buildHuffTree method on a corrupted file") {
    std::string test_str = {2, 'a', 1, 0, 0, 0, 'b', '~', '~'};
    std::istringstream decode_str(test_str, std::ios::binary);
    CHECK_THROWS_WITH_AS(huffman_archiver.decode_buildHuffTree(decode_str),
                         "File format error!", std::runtime_error);
  }

  SUBCASE("testing encode method") {
    std::ostringstream out(std::ios::binary);
    std::string test_str;
    std::string compare_str;

    SUBCASE("state situation") {
      test_str = "cbcacbc";
      compare_str = {2, 'a', 1, 0, 0, 0,
                        'b', 2, 0, 0, 0,
                        'c', 4, 0, 0, 0, 2,
                     static_cast<char>(0b01001101),
                     static_cast<char>(0b00000011)};
    }

    SUBCASE("file which consists of one repeating character") {
      test_str = "aaaaaaaaaa";
      compare_str = {0, 'a', 0xA, 0, 0, 0};
    }

    SUBCASE("file which consists of one repeating character") {
      test_str = {};
      compare_str = {};
    }

    SUBCASE("file which size (in bits) is divisible by 8") {
      test_str = "aaaabbbb";
      compare_str = {1, 'a', 4, 0, 0, 0,
                        'b', 4, 0, 0, 0, 0,
                     static_cast<char>(0b11110000)};
    }

    std::istringstream encode_str(test_str, std::ios::binary);
    REQUIRE_NOTHROW(huffman_archiver.encode(encode_str, out));
    CHECK_EQ(out.str(), compare_str);
  }

  SUBCASE("testing decode method") {
    std::ostringstream out(std::ios::binary);
    std::string test_str;
    std::string compare_str;

    SUBCASE("state situation") {
      test_str = {2, 'a', 1, 0, 0, 0,
                     'b', 2, 0, 0, 0,
                     'c', 4, 0, 0, 0, 2,
                     static_cast<char>(0b01001101),
                     static_cast<char>(0b00000011)};
      compare_str = "cbcacbc";
    }

    SUBCASE("file which consists of one repeating character") {
      test_str = {0, 'a', 10, 0, 0, 0};
      compare_str = "aaaaaaaaaa";
    }

    SUBCASE("empty file") {
      test_str = {};
      compare_str = {};
    }

    std::istringstream decode_str(test_str, std::ios::binary);
    REQUIRE_NOTHROW(huffman_archiver.decode(decode_str, out));
    CHECK_EQ(out.str(), compare_str);
  }

  SUBCASE("testing encode-decode together") {
    std::string test_str;
    SUBCASE("state situation") {
      test_str = "How great that everything runs smoothly!";
    }
    SUBCASE("file which consists of one repeating character") {
      test_str = "aaaaaaaaaa";
    }
    SUBCASE("empty file") {
      test_str = {};
    }
    SUBCASE("possible last-byte-missing error") {
      test_str = "aaaabbbb";
    }
    std::istringstream encode_str(test_str, std::ios::binary);
    std::ostringstream encoded_str(std::ios::binary);
    std::istringstream decode_str(std::ios::binary);
    std::ostringstream check_str(std::ios::binary);
    REQUIRE_NOTHROW(huffman_archiver.encode(encode_str, encoded_str));
    decode_str.str(encoded_str.str());
    REQUIRE_NOTHROW(huffman_archiver.decode(decode_str, check_str));
    CHECK_EQ(test_str, check_str.str());
  }
}
