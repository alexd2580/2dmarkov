
#include <iostream>

#include "png_image.hpp"

using namespace std;

PNG_image::PNG_image(string const& fname) : filename(fname) {}

PNG_image::PNG_image(string const& fname, PNG_image& ref) : filename(fname)
{
  height = ref.height;
  width = ref.width;

  rows = ref.rows;
  bytes = ref.bytes;
}

ostream& PNG_image::log(void)
{
  return cout << "[PNG_image] " << filename << ": ";
}

ostream& PNG_image::err(void)
{
  return cerr << "[PNG_image] " << filename << ": ";
}

void PNG_image::allocate(size_t h, size_t w, size_t w_bytes)
{
  log() << "Allocating memory for image: " << h << "*" << w << " ("
        << w_bytes / w << " bytes per pixel)" << endl;

  height = h;
  width = w;

  png_byte** row_ptr = new png_byte*[h];
  auto delpp = [filename = filename](png_byte * *ptr)
  {
    cout << "deleting row ptr of " << filename << endl;
    delete[] ptr;
  };
  rows = shared_ptr<png_byte*>(row_ptr, delpp);

  png_byte* byte_ptr = new png_byte[w_bytes * h];
  auto delp = [filename = filename](png_byte * ptr)
  {
    cout << "deleting byte ptr of " << filename << endl;
    delete[] ptr;
  };
  bytes = shared_ptr<png_byte>(byte_ptr, delp);
}

size_t PNG_image::get_height(void) { return height; }
size_t PNG_image::get_width(void) { return width; }

bool PNG_image::load(void)
{
  log() << "Loading image" << endl;
  FILE* fp = fopen(filename.c_str(), "rb");
  if(fp == nullptr)
  {
    err() << "File could not be opened for reading" << endl;
    return false;
  }

  png_struct* png_ptr =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if(png_ptr == nullptr)
  {
    err() << "Failed to create read struct" << endl;
    return false;
  }

  png_info* info_ptr = png_create_info_struct(png_ptr);
  if(info_ptr == nullptr)
  {
    err() << "Failed to create info struct" << endl;
    return false;
  }

  if(setjmp(png_jmpbuf(png_ptr)))
  {
    err() << "i don't know what that does TODO" << endl;
    return false;
  }

  png_init_io(png_ptr, fp);
  png_read_info(png_ptr, info_ptr);

  size_t w = png_get_image_width(png_ptr, info_ptr);
  size_t h = png_get_image_height(png_ptr, info_ptr);
  size_t bytes_per_row = png_get_rowbytes(png_ptr, info_ptr);

  allocate(h, w, bytes_per_row);

  png_byte** row_ptr = rows.get();
  png_byte* byte_ptr = bytes.get();

  for(size_t y = 0; y < height; y++)
    row_ptr[y] = byte_ptr + y * bytes_per_row;

  png_read_image(png_ptr, row_ptr);
  fclose(fp);
  return true;
}

RowPtr PNG_image::get_data(void) { return rows; }

bool PNG_image::unload(void)
{
  log() << "Unloading image" << endl;
  FILE* fp = fopen(filename.c_str(), "wb");
  if(fp == nullptr)
  {
    err() << "Could not open write file" << endl;
    return false;
  }

  png_structp png_wptr =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if(png_wptr == nullptr)
  {
    err() << "Failed to create write struct" << endl;
    return false;
  }

  png_infop info_wptr = png_create_info_struct(png_wptr);
  if(info_wptr == nullptr)
  {
    err() << "Failed to create info struct" << endl;
    return false;
  }

  if(setjmp(png_jmpbuf(png_wptr)))
  {
    err() << "i don't know what that does TODO" << endl;
    return false;
  }

  png_init_io(png_wptr, fp);

  // Output is 8bit depth, RGBA format.
  png_set_IHDR(png_wptr,
               info_wptr,
               (png_uint_32)width,
               (png_uint_32)height,
               8,
               PNG_COLOR_TYPE_RGBA,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png_wptr, info_wptr);

  // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
  // Use png_set_filler().
  // png_set_filler(png, 0, PNG_FILLER_AFTER);

  png_write_image(png_wptr, rows.get());
  png_write_end(png_wptr, nullptr);
  fclose(fp);

  return true;
}

bool file_exists(const string& fname)
{
  FILE* file = fopen(fname.c_str(), "r");
  if(!file)
    return false;
  fclose(file);
  return true;
}

string PNG_image::next_free_filename(string const& fname)
{
  string name = fname;

  if(fname.length() >= 5)
  {
    size_t splitpoint = fname.length() - 4;
    string ext = fname.substr(splitpoint);
    if(ext.compare(".PNG") == 0 || ext.compare(".png") == 0)
      name = fname.substr(0, splitpoint);
  }

  int suffix = 0;
  string new_name = name + "_" + std::to_string(suffix) + ".png";
  while(file_exists(new_name))
  {
    suffix++;
    new_name = name + "_" + std::to_string(suffix) + ".png";
  }

  return new_name;
}
