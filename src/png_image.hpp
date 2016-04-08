
#include <memory>
#include <ostream>
#include <png.h>
#include <string>

using RowPtr = std::shared_ptr<png_byte*>;
using DataPtr = std::shared_ptr<png_byte>;

//, std::function<void(png_byte*)

class PNG_image
{
private:
  size_t height;
  size_t width;

  RowPtr rows;
  DataPtr bytes;

  std::ostream& log(void);
  std::ostream& err(void);

public:
  std::string const filename;

  /**
   * Creates a new empty PNG stub.
   */
  explicit PNG_image(std::string const&);

  /**
   * Creates a PNG object with the dimension and shared memory of @ref.
   */
  explicit PNG_image(std::string const&, PNG_image& ref);

  /**
   * Allocates memory.
   */
  void allocate(size_t h, size_t w, size_t w_bytes);

  size_t get_height(void);
  size_t get_width(void);

  /**
   * Allocates enough memory and loads the png file into it.
   * Returns false on error.
   */
  bool load(void);
  RowPtr get_data(void);

  /**
   * Writes the shared memory to a PNG file.
   * Returns false on error.
   */
  bool unload(void);

  virtual ~PNG_image(void) = default;

  static std::string next_free_filename(std::string const&);
};
