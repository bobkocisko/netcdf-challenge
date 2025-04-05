#include "crow.h"
#include "netcdf"

using namespace netCDF;
using namespace crow;

class read_netcdf {
  const NcFile& file;

  int format;

  public:

  read_netcdf(const NcFile& file)
    : file(file)
  {
    // Apparently you have to drop down to the c library directly
    // to get access to the format information
    // https://github.com/Unidata/netcdf-cxx4/issues/49
    nc_inq_format(file.getId(), &this->format);
  }

  json::wvalue get_info() const {
    return get_info(file);    
  }

  private:

  /**
   * Why do we iterate in reverse?  Because for some reason the
   * crow json api insists on presenting the fields in
   * the reverse order that they were inserted when a multimap
   * is involved.
   */
  template <typename K, typename V>
  auto iterate_in_reverse(const std::multimap<K, V>& map) const {
    auto o = json::wvalue::object();
    for (auto iter = map.rbegin(); iter != map.rend(); ++iter) {
      auto &[k, v] = *iter;
      o[k] = get_info(v);
    }
    return o;
  }

  json::wvalue get_info(const NcGroup& g) const {
    auto w = json::wvalue::object();

    // NOTE: Writing these in the reverse order on purpose to fix
    // crow json's ordering

    w["attributes"] = iterate_in_reverse(g.getAtts());
    w["variables"] = iterate_in_reverse(g.getVars());
    w["dimensions"] = iterate_in_reverse(g.getDims());

    return w;
  }

  json::wvalue get_info(const NcAtt& a) const {
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

  json::wvalue get_info(const NcDim& d) const {
    if (d.isUnlimited()) {
      return "<unlimited>";
    }
    return d.getSize();
  }

  json::wvalue get_info(const NcVar& v) const {
    auto w = json::wvalue::object();

    // Variable attributes (for some reason *these*
    // end up in the correct order so we don't reverse them)
    {
      auto ga = json::wvalue::object();
      for (auto &[k, val] : v.getAtts())
      {
        ga[k] = get_info(val);
      }
      w["attributes"] = std::move(ga);
    }

    // Variable dimensions (just an array of names)
    {
      auto vd = json::wvalue::list();
      for (auto& d: v.getDims()) {
        vd.push_back(d.getName());
      }
      w["dimensions"] = std::move(vd);
    }

    return w;
  }

};