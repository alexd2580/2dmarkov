#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "pblt.hpp"
#include "png_image.hpp"

using namespace std;

ostream& log(void) { return cout << "[MAIN] "; }

ostream& err(void) { return cerr << "[MAIN] "; }

using Succ_node = PBLT_node<uint32_t>;
using Succ_ref = PBLT_ref<uint32_t>;

uint32_t get_pixel_from_row(png_byte* row, size_t x)
{
  png_byte c_r = row[x * 4 + 0];
  png_byte c_g = row[x * 4 + 1];
  png_byte c_b = row[x * 4 + 2];
  return (uint32_t)(c_r << 24 | c_g << 16 | c_b << 8 | 255);
}

/*void analyze_image(PNG_image& src, Succ_node& tree_root)
{
  auto height = src.get_height();
  auto width = src.get_width();

  auto shared_rows = src.get_data();
  auto raw_rows = shared_rows.get();

  deque<Succ_ref> insert_nodes;
  size_t const MAX_SIZE = 5;

  for(size_t y = 0; y < height; y++)
  {
    insert_nodes.clear();
    insert_nodes.push_back(Succ_ref(tree_root));

    png_byte* row = raw_rows[y];
    for(size_t x = 0; x < width; x++)
    {
      uint32_t color = get_pixel_from_row(row, x);

      size_t size = insert_nodes.size();
      for(size_t i = 0; i < size; i++)
      {
        Succ_ref& r = insert_nodes.front();
        insert_nodes.pop_front();
        Succ_node& nxt = r.visit(color & 0b11111000111110001111100011111000);
        insert_nodes.push_back(Succ_ref(nxt.get_successors()));
      }
      insert_nodes.push_front(Succ_ref(tree_root));
      if(insert_nodes.size() > MAX_SIZE)
        insert_nodes.pop_back();
    }
  }
}*/

//#define SINNVOLLE_PIXEL(x) (x & 0b11111000111110001111100011111000)
#define SINNVOLLE_PIXEL(x) (x & 0b11111111111111111111111111111111)

bool dist_greater_than(uint32_t a, uint32_t b, uint32_t t)
{
  uint32_t d = 0;
  uint32_t ca, cb;
  ca = a >> 24 & 0xFF;
  cb = b >> 24 & 0xFF;
  d += ca > cb ? ca - cb : cb - ca;
  ca = a >> 16 & 0xFF;
  cb = b >> 16 & 0xFF;
  d += ca > cb ? ca - cb : cb - ca;
  ca = a >> 8 & 0xFF;
  cb = b >> 8 & 0xFF;
  d += ca > cb ? ca - cb : cb - ca;
  return d > t;
}

void analyze_image(PNG_image& src, Succ_node& tree_root)
{
  auto height = src.get_height();
  auto width = src.get_width();

  auto shared_rows = src.get_data();
  auto raw_rows = shared_rows.get();

  Succ_ref root(tree_root);

  uint32_t treshold = 100;

  for(size_t y = 0; y < height; y++)
  {
    png_byte* row = raw_rows[y];
    for(size_t x = 0; x < width; x++)
    {
      uint32_t color = get_pixel_from_row(row, x);
      Succ_node& node = root.visit(SINNVOLLE_PIXEL(color));
      Succ_node& succs = node.get_successors();

      if(y > 0)
      {
        if(x > 0)
        {
          uint32_t c1 = get_pixel_from_row(raw_rows[y - 1], x - 1);
          if(dist_greater_than(color, c1, treshold))
            succs.visit(SINNVOLLE_PIXEL(c1));
        }
        uint32_t c2 = get_pixel_from_row(raw_rows[y - 1], x);
        if(dist_greater_than(color, c2, treshold))
          succs.visit(SINNVOLLE_PIXEL(c2));
        if(x < width - 1)
        {
          uint32_t c3 = get_pixel_from_row(raw_rows[y - 1], x + 1);
          if(dist_greater_than(color, c3, treshold))
            succs.visit(SINNVOLLE_PIXEL(c3));
        }
      }

      if(x > 0)
      {
        uint32_t c1 = get_pixel_from_row(row, x - 1);
        if(dist_greater_than(color, c1, treshold))
          succs.visit(SINNVOLLE_PIXEL(c1));
      }
      if(x < width - 1)
      {
        uint32_t c3 = get_pixel_from_row(row, x + 1);
        if(dist_greater_than(color, c3, treshold))
          succs.visit(SINNVOLLE_PIXEL(c3));
      }

      if(y < height - 1)
      {
        if(x > 0)
        {
          uint32_t c1 = get_pixel_from_row(raw_rows[y + 1], x - 1);
          if(dist_greater_than(color, c1, treshold))
            succs.visit(SINNVOLLE_PIXEL(c1));
        }
        uint32_t c2 = get_pixel_from_row(raw_rows[y + 1], x);
        if(dist_greater_than(color, c2, treshold))
          succs.visit(SINNVOLLE_PIXEL(c2));
        if(x < width - 1)
        {
          uint32_t c3 = get_pixel_from_row(raw_rows[y + 1], x + 1);
          if(dist_greater_than(color, c3, treshold))
            succs.visit(SINNVOLLE_PIXEL(c3));
        }
      }
    }
  }
}

struct PixelStatus
{
  bool done = false;
  bool queued = false;
  deque<Succ_ref> neighbours;
};

bool inside_image(int32_t x, int32_t y, int32_t w, int32_t h)
{
  return x >= 0 && x < w && y >= 0 && y < h;
}

auto& lookup_pixel(map<uint32_t, PixelStatus>& pixels, uint32_t x, uint32_t y)
{
  uint32_t id = x << 16 | y << 0;
  auto pixel =
      pixels.insert(std::pair<uint32_t, PixelStatus>(id, PixelStatus()));
  return *pixel.first;
}

// template <typename T> class TD;

void update_todo_pixel(map<uint32_t, PixelStatus>& pixels,
                       uint32_t x,
                       uint32_t y,
                       uint32_t width,
                       uint32_t height,
                       deque<uint32_t>& todo,
                       Succ_node& node_from_root)
{
  if(inside_image((int32_t)x, (int32_t)y, (int32_t)width, (int32_t)height))
  {
    auto& pix_pair = lookup_pixel(pixels, x, y);
    uint32_t pix_key = pix_pair.first;
    auto& pix_stat = pix_pair.second;
    if(!pix_stat.done)
    {
      pix_stat.neighbours.push_back(Succ_ref(node_from_root));
      if(!pix_stat.queued)
      {
        todo.push_back(pix_key);
        pix_stat.queued = true;
      }
    }
  }
}

void generate_image(Succ_node& tree_root, PNG_image& dst)
{
  map<uint32_t, PixelStatus> pixels;
  deque<uint32_t> todo;

  uint32_t height = (uint32_t)dst.get_height();
  uint32_t width = (uint32_t)dst.get_width();

  for(int i = 0; i < 100; i++)
  {
    uint32_t start_x = (uint32_t)rand() % width;
    uint32_t start_y = (uint32_t)rand() % height;

    uint32_t start_key = start_x << 16 | start_y;
    todo.push_back(start_key);
    auto& start_pair = lookup_pixel(pixels, start_x, start_y);
    start_pair.second.neighbours.push_back(Succ_ref(tree_root));
    start_pair.second.queued = true;
  }

  auto shared_rows = dst.get_data();
  auto raw_rows = shared_rows.get();

  while(todo.size() > 0)
  {
    uint32_t f = todo.front();
    todo.pop_front();
    PixelStatus& status = pixels.at(f);

    uint32_t x = f >> 16 & 0xFFFF;
    uint32_t y = f >> 0 & 0xFFFF;

    while(status.neighbours.size() > 1)
    {
      Succ_node& a = status.neighbours.front().get_node();
      status.neighbours.pop_front();
      Succ_node& b = status.neighbours.front().get_node();
      status.neighbours.pop_front();

      status.neighbours.push_back(
          Succ_ref(Succ_node::select_random(a, b, (uint32_t)rand())));
    }

    Succ_node& node =
        status.neighbours.front().get_node().find_random((uint32_t)rand());
    uint32_t color = node.get_key();

    png_byte* row = raw_rows[y];
    png_byte* pix = row + x * 4;
    pix[0] = color >> 24 & 0xFF;
    pix[1] = color >> 16 & 0xFF;
    pix[2] = color >> 8 & 0xFF;
    pix[3] = 255;

    Succ_node& node_from_root = tree_root.find(color);

    update_todo_pixel(pixels, x - 1, y, width, height, todo, node_from_root);
    update_todo_pixel(pixels, x + 1, y, width, height, todo, node_from_root);
    update_todo_pixel(pixels, x, y - 1, width, height, todo, node_from_root);
    update_todo_pixel(pixels, x, y + 1, width, height, todo, node_from_root);

    status.done = true;
  }
}

int main(int argc, char* argv[])
{
  srand((unsigned int)time(nullptr));
  /*for(int i = 0; i < 100; i++)
  {
    auto& res = PBLT<uint32_t, PixelInfo>::visit(tree, 42);
    res += 10;
  }

  for(int i = 0; i < 1000; i++)
    cout << tree->find_random((uint32_t)rand()) << endl;*/

  if(argc < 2)
  {
    log() << "Please supply image filename ____.png" << endl;
    return 1;
  }

  string filename(argv[1]);

  PNG_image source(filename + ".png");
  if(!source.load())
    return 1;

  Succ_node tree_root;
  log() << "Analyzing" << endl;
  analyze_image(source, tree_root);
  PNG_image destination(PNG_image::next_free_filename(filename), source);
  log() << "Generating" << endl;
  generate_image(tree_root, destination);
  destination.unload();

  log() << "Exit" << endl;
}
