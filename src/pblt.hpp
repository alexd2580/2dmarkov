#include <iostream>
#include <memory>

/**
 * Probabilistic binary tree.
 *
 * Ord Key => typename Key : type of key (efficiently copy-constructible)
 * typename Value : type of value stored at the nodes (should be
 * move-constructible)
 */

template <typename Key> class PBLT_node
{

public:
  using Ptr = std::shared_ptr<PBLT_node<Key>>;

private:
  uint32_t total_count;

  Ptr left;
  uint32_t left_count;

  uint32_t this_count;
  Ptr successors;

  Ptr right;
  uint32_t right_count;

public:
  Key const key;

  PBLT_node(Key k) : successors(), key(k)
  {
    this_count = total_count = 1;
    left_count = right_count = 0;
  }

  virtual ~PBLT_node(void) = default;

  Ptr visit(Ptr this_, Key k)
  {
    total_count++;
    if(k == key)
    {
      this_count++;
      return this_;
    }
    if(k < key)
    {
      left_count++;
      if(!left)
      {
        left = std::make_shared<PBLT_node<Key>>(k);
        return left;
      }
      return left->visit(left, k);
    }
    else
    {
      right_count++;
      if(!right)
      {
        right = std::make_shared<PBLT_node<Key>>(k);
        return right;
      }
      else
        return right->visit(right, k);
    }
  }

  Ptr find_random(uint32_t rnd)
  {
    rnd %= total_count;
    /*std::cout << "rnd " << rnd << " leftc " << left_count << " rightc "
              << right_count << " thisc " << this_count << " totc "
              << total_count << std::endl;*/
    if(rnd < left_count)
      return left->find_random(rnd);
    else if(rnd - left_count < this_count)
      return successors;
    else
      return right->find_random(rnd);
  }

  void print(int depth)
  {
    using namespace std;
    for(int i = 0; i < depth - 1; i++)
      cout << "| ";
    cout << "+------" << endl;
    for(int i = 0; i < depth - 1; i++)
      cout << "| ";
    cout << "| total: " << total_count << endl;
    for(int i = 0; i < depth - 1; i++)
      cout << "| ";
    cout << "| key: " << (int)key << endl;
    for(int i = 0; i < depth - 1; i++)
      cout << "| ";
    cout << "| left: " << left_count << endl;
    for(int i = 0; i < depth - 1; i++)
      cout << "| ";
    cout << "| right: " << right_count << endl;
    for(int i = 0; i < depth - 1; i++)
      cout << "| ";
    cout << "+-+----" << endl;

    if(right)
    {
      right->print(depth + (left ? 1 : 0));
    }
    if(left)
    {
      left->print(depth);
    }
  }
};

template <typename Key> class PBLT
{
  using Node_Ptr = typename PBLT_node<Key>::Ptr;
  Node_Ptr node;

public:
  explicit PBLT(void) = default;
  explicit PBLT(Node_Ptr a) : node(a) {}
  ~PBLT(void) = default;

  std::unique_ptr<PBLT> copy_ptr(void) { return std::make_unique<PBLT>(node); }

  void print(void)
  {
    using namespace std;
    if(!node)
      cout << "empty" << endl;
    else
      node->print(0);
  }

  /**
   * Inserts a new element. Increments counter by one
   */
  std::unique_ptr<PBLT> visit(Key k)
  {
    if(!node)
    {
      node.reset(new PBLT_node<Key>(k));
      return std::make_unique<PBLT>(node);
    }
    return std::make_unique<PBLT>(node->visit(node, k));
  }

  PBLT<Key> find_random(uint32_t rnd) { return PBLT(node->find_random(rnd)); }
};
