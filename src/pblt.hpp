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
  using Ptr = std::unique_ptr<PBLT_node<Key>>;

private:
  bool online;

  uint32_t total_count;

  Ptr left;
  uint32_t left_count;

  Key key;
  Ptr successors;
  uint32_t this_count;

  Ptr right;
  uint32_t right_count;

public:
  PBLT_node() : key(), successors()
  {
    online = false;
    this_count = total_count = 0;
    left_count = right_count = 0;
  }

  virtual ~PBLT_node(void) = default;

  bool is_empty(void) const { return !online; }

  PBLT_node& visit(Key k)
  {
    total_count++;
    if(online)
    {
      if(k == key)
      {
        this_count++;
        return *this;
      }
      if(k < key)
      {
        left_count++;
        return left->visit(k);
      }
      else
      {
        right_count++;
        return right->visit(k);
      }
    }
    else
    {
      online = true;
      key = k;
      this_count++;
      left = std::make_unique<PBLT_node<Key>>();
      right = std::make_unique<PBLT_node<Key>>();
      return *this;
    }
  }

  PBLT_node& find_random(uint32_t rnd)
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
    cout << "| this: " << this_count << endl;
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

    if(right->online)
    {
      right->print(depth + 1);
    }
    if(left->online)
    {
      left->print(depth);
    }
  }
};

template <typename Key> class PBLT_ref
{
private:
  PBLT_node<Key>& pblt_node;

public:
  PBLT_ref(void) = delete;
  explicit PBLT_ref(PBLT_node<Key>& node_ref) : pblt_node(node_ref) {}
  explicit PBLT_ref(PBLT_ref const& a) : pblt_node(a.pblt_node) {}
  ~PBLT_ref(void) = default;

  void print(void)
  {
    using namespace std;
    if(pblt_node.is_empty())
      cout << "empty" << endl;
    else
      pblt_node.print(0);
  }

  /**
   * Inserts a new element. Increments counter by one
   */
  PBLT_node<Key>& visit(Key k) { return pblt_node.visit(k); }

  PBLT_node<Key>& find_random(uint32_t rnd)
  {
    return pblt_node.find_random(rnd);
  }
};
