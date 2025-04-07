#include <nlohmann/json.hpp>
#include <netcdf>

using namespace netCDF;
using json = nlohmann::ordered_json;

class read_netcdf {
  NcFile file;
  int format;

public:
  read_netcdf(const char *path)
    : file(path, NcFile::FileMode::read)
  {
    // Apparently you have to drop down to the c library directly
    // to get access to the format information
    // https://github.com/Unidata/netcdf-cxx4/issues/49
    nc_inq_format(file.getId(), &this->format);
  }

  /**
   * Returns general information about dimensions, variables and
   * attributes formatted as json
   */
  json get_info() const {
    return get_info(file);    
  }

  /**
   * Returns the data as a json document for the specified variable_name
   * without any dimension constraints.
   */
  json get_data(const char* variable_name) {
    return get_data(variable_name, std::vector<uint64_t>());
  }
  /**
   * Returns the data as a json document for the specified variable_name with
   * its first dimensions constrained to the specified values in 'prefix_indices'
   */
  json get_data(
      const char* variable_name, 
      std::vector<uint64_t> prefix_indices
  ) const {
    NcVar var = file.getVar(variable_name);
    try {
      if (var.isNull()) {
        throw std::invalid_argument("does not exist");
      }
      if (prefix_indices.size() > var.getDimCount()) {
        throw std::invalid_argument("has " + 
          std::to_string(var.getDimCount()) + " dimensions " +
          "but you've specifed more indexes (" + 
          std::to_string(prefix_indices.size()) + ")");
      }
    }
    catch(std::exception& e) {
      throw std::invalid_argument(
        std::string("Variable name '") + variable_name + "': " + e.what());
    }

    // The first dimensions we'll start at the indices in 'prefix_indices'
    // and do counts of 1
    std::vector<uint64_t> indices = prefix_indices;
    std::vector<uint64_t> counts(prefix_indices.size(), 1);

    std::size_t first_unrestricted_index = prefix_indices.size();
    // Then we want to add the entire range [0, size) of the
    // remaining dimensions
    std::size_t total_data_elements = 1;
    for (std::size_t i = first_unrestricted_index; i < var.getDimCount(); ++i)
    {
      indices.push_back(0);
      NcDim dim = var.getDim(i);
      counts.push_back(dim.getSize());
      total_data_elements *= dim.getSize();
    }


    std::size_t buffer_size = total_data_elements * var.getType().getSize();
    std::vector<char> buf(buffer_size);
    void* buffer = (void*)buf.data();
    var.getVar(indices, counts, buffer);

    std::vector<json> lists(indices.size());

    std::size_t last_dim_index = var.getDimCount() - 1;

    // Handle special case where all indices have been specified
    if (first_unrestricted_index == indices.size()) {
      return get_data_from_buffer(var.getType(), buffer, 0);
    }

    for (std::size_t i = 0; i < total_data_elements; ++i) {
      for (
          std::size_t li = last_dim_index; 
          li >= first_unrestricted_index;
          --li
      ) {
        json &dim_list = lists[li];
        if (li == last_dim_index) {
          dim_list.push_back(
            get_data_from_buffer(var.getType(), buffer, i));
        } 

        if (dim_list.size() == counts[li]) {
          if (li > first_unrestricted_index) {
            // It's time to append this list to the previous one
            lists[li - 1].push_back(dim_list);
            dim_list.clear();
          }
        }
        else {
          // We can break out of the for loop because we're
          // still filling up this list.
          break;
        }
      }
    }
    // Return the root unbounded list
    return lists[first_unrestricted_index];
  }


  /**
   * Validates that the specified index is valid for the dimension,
   * and throws exception if either the dimension name or index are
   * invalid.
   */
  void validate_dimension_index(
      const char* dimension_name, 
      std::size_t attempting_index
  ) const {
    NcDim dim = file.getDim(dimension_name);
    if (dim.isNull()) {
      throw std::invalid_argument(
        std::string("No such dimension '") + dimension_name + "'");
    }
    if (attempting_index >= dim.getSize()) {
      throw std::invalid_argument(
        std::string("Dimension '") + dimension_name + 
        " has size " + std::to_string(dim.getSize()) + 
        " which means your attempted index " + 
        std::to_string(attempting_index) + " is invalid");
    }
  }

private:

  /**
   * Return the json value depending on the type of data for the specified 
   * type, reading from the buffer at the specified index. 
   * NOTE: This should be merged with `get_info(const NcAtt& a)` below 
   * since they do similar things, but in the interest of time
   * for now I'm keeping them separate
   */
  json get_data_from_buffer(
      NcType t, void* buffer, std::size_t index) const 
  {
    switch (t.getTypeClass())
    {
      case NcType::nc_DOUBLE:
        return ((double *)buffer)[index];
      default:
        // NOTE: we should obviously extend this to handle all possible
        // types but I have run out of time to complete this so we're
        // throwing an exception if another type is encountered.
        throw std::invalid_argument(
          "WIP - Currently only handles DOUBLE values");
    }
  }

  /**
   * Call get_info for each value in the specified multimap.
   * Why do we iterate in reverse?  Because for some reason the
   * crow json api insists on presenting the fields in
   * the reverse order that they were inserted when a multimap
   * is involved.
   * NOTE! This implementation only supports a single value
   * for each key in the multimap, which seems to be sufficient
   * for the sample file provided.  A future upgrade could
   * convert the first value to a list upon discovery of a second.
   */
  template <typename K, typename V>
  auto get_info_mm(const std::multimap<K, V>& map) const {
    auto o = json::object();
    for (auto& [k, v]: map) {
      o[k] = get_info(v);
    }
    return o;
  }

  json get_info(const NcGroup& g) const {
    return {
      {"dimensions", get_info_mm(g.getDims())},
      {"variables", get_info_mm(g.getVars())},
      {"attributes", get_info_mm(g.getAtts())},
    };
  }

  json get_info(const NcAtt& a) const {
    if (a.isNull()) {
      return nullptr;
    }

    NcType t = a.getType();

    // NOTE!  This is a minimal implementation which should be
    //  extended to support multiple values per attribute.  For
    //  now it simply takes the first value available, which 
    //  appears to be sufficient to process the provided sample file.

    std::vector<char> buffer(a.getAttLength() * a.getType().getSize());
    void* data = buffer.data();
    a.getValues(data);

    switch(t.getTypeClass()) {
      case NcType::nc_BYTE:     //!< signed 1 byte integer
      {
        return ((signed char*)data)[0];
      }
      case NcType::nc_CHAR:     //!< ISO/ASCII character
      {
        // Contrary to the description with this enum element,
        // this is actually a char*, ie a string, and it needs
        // some very special attention.
        // I found this by taking a look at the ncdump source
        // https://github.com/Unidata/netcdf-c/blob/main/ncdump/ncdump.c#L414
        return get_attr_string(buffer);
      }
      case NcType::nc_SHORT:    //!< signed 2 byte integer
      {        
        return ((short *)data)[0];
      }
      case NcType::nc_INT:      //!< signed 4 byte integer
      {        
        return ((int *)data)[0];
      }
      case NcType::nc_FLOAT:    //!< single precision floating point number
      {        
        return ((float *)data)[0];
      }
      case NcType::nc_DOUBLE:   //!< double precision floating point number
      {        
        return ((double *)data)[0];
      }
      case NcType::nc_UBYTE:    //!< unsigned 1 byte int
      {        
        return ((unsigned char *)data)[0];
      }
      case NcType::nc_USHORT:   //!< unsigned 2-byte int
      {        
        return ((unsigned short*)data)[0];
      }
      case NcType::nc_UINT:     //!< unsigned 4-byte int
      {
        return ((unsigned int*)data)[0];
      }
      case NcType::nc_INT64:    //!< signed 8-byte int
      {        
        return ((int64_t*)data)[0];
      }
      case NcType::nc_UINT64:   //!< unsigned 8-byte int
      {     
        return ((uint64_t*)data)[0];
      }
      case NcType::nc_STRING:   //!< string
      {        
        char** strings = (char **)data;
        char* s = strings[0];
        if (s == nullptr) {
          return nullptr;
        } 
        size_t len = strlen(s);
        std::vector<char> buf(len);
        strncpy(buf.data(), s, len);
        // NOTE: we need to free the string buffers that
        // have been created for us after we copy the data
        // to our own buffer:
        // https://docs.unidata.ucar.edu/netcdf-c/4.9.3/group__attributes.html#ga19cae92a58e1bf7f999c3eeab5404189
        nc_free_string(a.getAttLength(), strings);
        return get_attr_string(buf);
      }
      case NcType::nc_VLEN:     //!< "NcVlen type"
      {        
        return "udf VLEN";
      }
      case NcType::nc_OPAQUE:   //!< "NcOpaque type"
      {
        return "udf OPAQUE";
      }
      case NcType::nc_ENUM:     //!< "NcEnum type"
      {
        return "udf ENUM";
      }
      case NcType::nc_COMPOUND: //!< "NcCompound type"
      {
        return "udf COMPOUND";
      }
      default:
        throw std::runtime_error("unexpected type " + 
          std::to_string(t.getTypeClass()));
    }
  }

  /**
   * Process nc_CHAR types differently, following
   * https://github.com/Unidata/netcdf-c/blob/main/ncdump/ncdump.c#L414
   */
  std::string get_attr_string(std::vector<char>& buf) const {
    // 1. Remove all trailing nulls
    auto iter = std::find_if(buf.rbegin(), buf.rend(), 
      [](auto tc){ return tc != '\0'; });
    // NOTE we are *not* using std::next() to increment
    // the iterator before converting to base, and that
    // is because the iterator we received is pointing
    // to the first non-zero character and we don't want
    // to delete that one.
    buf.erase(iter.base(), buf.end());

    // 2. We need to replace control characters with readable
    //   equivalents

    std::stringstream result;

    for (auto c: buf) {
      // I have no idea why we're bit-masking this but I'm
      // following the code from ncdump.
      unsigned char uc = (unsigned char)(c & 0377);
      switch (uc)
      {
        case '\b':
          result << "\\b";
          break;
        case '\f':
          result << "\\f";
          break;
        case '\n':
          /* Only generate linebreaks after embedded newlines for
          * classic, 64-bit offset, cdf5, or classic model files.  For
          * netCDF-4 files, don't generate linebreaks, because that
          * would create an extra string in a list of strings.  */
          if (format != NC_FORMAT_NETCDF4)
          {
            // NOTE: technically we should be returning here an
            // array of strings, but for now I'm keeping it simple
            // and just keeping things similar to the ncdump approach
            result << "\\n\",\"";
          }
          else
          {
            result << "\\n";
          }
          break;
        case '\r':
          result << "\\r";
          break;
        case '\t':
          result << "\\t";
          break;
        case '\v':
          result << "\\v";
          break;
        case '\\':
          result << "\\\\";
          break;
        case '\'':
          result << "\\\'";
          break;
        case '\"':
          result << "\\\"";
          break;
        default:
          if (iscntrl(uc)) {
            std::ios init(nullptr);
            init.copyfmt(result);
            result 
              << std::setfill('0') << std::setw(2) 
              << std::oct << uc;
            // Restore initial formatting
            result.copyfmt(init);
          }
          else
            result << (char)uc;
          break;
      }
    }
    return result.str();
  }

  json get_info(const NcDim& d) const {
    if (d.isUnlimited()) {
      return "<unlimited>";
    }
    return d.getSize();
  }

  json get_info(const NcVar& v) const {
    // Variable attributes
    auto ga = json::object();
    for (auto &[k, val] : v.getAtts())
    {
      ga[k] = get_info(val);
    }

    // Variable dimensions (just an array of names)
    auto vd = json::array();
    for (auto& d: v.getDims()) {
      vd.push_back(d.getName());
    }

    return {
      {"dimensions", vd},
      {"attributes", ga}
    };
  }



};