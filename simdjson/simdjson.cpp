/* auto-generated on Tue  9 Jul 2019 16:39:33 EDT. Do not edit! */
#include "simdjson.h"

/* used for http://dmalloc.com/ Dmalloc - Debug Malloc Library */
#ifdef DMALLOC
#include "dmalloc.h"
#endif

/* begin file src/simdjson.cpp */
#include <map>

namespace simdjson {
const std::map<int, const std::string> errorStrings = {
    {SUCCESS, "No errors"},
    {CAPACITY, "This ParsedJson can't support a document that big"},
    {MEMALLOC, "Error allocating memory, we're most likely out of memory"},
    {TAPE_ERROR, "Something went wrong while writing to the tape"},
    {STRING_ERROR, "Problem while parsing a string"},
    {T_ATOM_ERROR, "Problem while parsing an atom starting with the letter 't'"},
    {F_ATOM_ERROR, "Problem while parsing an atom starting with the letter 'f'"},
    {N_ATOM_ERROR, "Problem while parsing an atom starting with the letter 'n'"},
    {NUMBER_ERROR, "Problem while parsing a number"},
    {UTF8_ERROR, "The input is not valid UTF-8"},
    {UNITIALIZED, "Unitialized"},
    {EMPTY, "Empty"},
    {UNESCAPED_CHARS, "Within strings, some characters must be escapted, we found unescapted characters"},
    {UNEXPECTED_ERROR, "Unexpected error, consider reporting this problem as you may have found a bug in simdjson"},
};

const std::string& errorMsg(const int errorCode) {
    return errorStrings.at(errorCode);
}
}
/* end file src/simdjson.cpp */
/* begin file src/jsonioutil.cpp */
#include <cstring>
#include <cstdlib>

namespace simdjson {
char * allocate_padded_buffer(size_t length) {
    // we could do a simple malloc
    //return (char *) malloc(length + SIMDJSON_PADDING);
    // However, we might as well align to cache lines...
    size_t totalpaddedlength = length + SIMDJSON_PADDING;
    char *padded_buffer = aligned_malloc_char(64, totalpaddedlength);
    return padded_buffer;
}

padded_string get_corpus(const std::string& filename) {
  std::FILE *fp = std::fopen(filename.c_str(), "rb");
  if (fp != nullptr) {
    std::fseek(fp, 0, SEEK_END);
    size_t len = std::ftell(fp);
    padded_string s(len);
    if(s.data() == nullptr) {
      std::fclose(fp);
      throw  std::runtime_error("could not allocate memory");
    }
    std::rewind(fp);
    size_t readb = std::fread(s.data(), 1, len, fp);
    std::fclose(fp);
    if(readb != len) {
      throw  std::runtime_error("could not read the data");
    }
    return s;
  }
  throw  std::runtime_error("could not load corpus");
}
}
/* end file src/jsonioutil.cpp */
/* begin file src/jsonparser.cpp */
#ifdef _MSC_VER
#include <windows.h>
#include <sysinfoapi.h>
#else
#include <unistd.h>
#endif

namespace simdjson {
// Responsible to select the best json_parse implementation
int json_parse_dispatch(const uint8_t *buf, size_t len, ParsedJson &pj, bool reallocifneeded) {
  // Versions for each implementation
#ifdef __AVX2__
  json_parse_functype* avx_implementation = &json_parse_implementation<instruction_set::avx2>;
#endif
#if defined(__SSE4_2__) || (defined(_MSC_VER) && defined(_M_AMD64))
  json_parse_functype* sse4_2_implementation = &json_parse_implementation<instruction_set::sse4_2>;
#endif
#if  defined(__ARM_NEON) || (defined(_MSC_VER) && defined(_M_ARM64))
  json_parse_functype* neon_implementation = &json_parse_implementation<instruction_set::neon>;
#endif

  // Determining which implementation is the more suitable
  // Should be done at runtime. Does not make any sense on preprocessor.
#ifdef __AVX2__
  instruction_set best_implementation = instruction_set::avx2;
#elif defined (__SSE4_2__) || (defined(_MSC_VER) && defined(_M_AMD64))
  instruction_set best_implementation = instruction_set::sse4_2;
#elif defined (__ARM_NEON) || (defined(_MSC_VER) && defined(_M_ARM64))
  instruction_set best_implementation = instruction_set::neon;
#else
  instruction_set best_implementation = instruction_set::none;
#endif
  
  // Selecting the best implementation
  switch (best_implementation) {
#ifdef __AVX2__
  case instruction_set::avx2 :
    json_parse_ptr = avx_implementation;
    break;
#endif
#if defined(__SSE4_2__) || (defined(_MSC_VER) && defined(_M_AMD64))
  case instruction_set::sse4_2 :
    json_parse_ptr = sse4_2_implementation;
    break;
#endif
#if defined(__ARM_NEON) || (defined(_MSC_VER) && defined(_M_ARM64))
  case instruction_set::neon :
    json_parse_ptr = neon_implementation;
    break;
#endif
  default :
    std::cerr << "No implemented simd instruction set supported" << std::endl;
    return simdjson::UNEXPECTED_ERROR;
  }

  return json_parse_ptr(buf, len, pj, reallocifneeded);
}

json_parse_functype *json_parse_ptr = &json_parse_dispatch;

WARN_UNUSED
ParsedJson build_parsed_json(const uint8_t *buf, size_t len, bool reallocifneeded) {
  ParsedJson pj;
  bool ok = pj.allocateCapacity(len);
  if(ok) {
    json_parse(buf, len, pj, reallocifneeded);
  } else {
    std::cerr << "failure during memory allocation " << std::endl;
  }
  return pj;
}
}
/* end file src/jsonparser.cpp */
/* begin file src/stage1_find_marks.cpp */
// File kept in case we want to reuse it soon. (many configuration files to edit)
/* end file src/stage1_find_marks.cpp */
/* begin file src/stage2_build_tape.cpp */
// File kept in case we want to reuse it soon. (many configuration files to edit)
/* end file src/stage2_build_tape.cpp */
/* begin file src/parsedjson.cpp */

namespace simdjson {
ParsedJson::ParsedJson() : 
        structural_indexes(nullptr), tape(nullptr), containing_scope_offset(nullptr),
        ret_address(nullptr), string_buf(nullptr), current_string_buf_loc(nullptr) {}

ParsedJson::~ParsedJson() {
    deallocate();
}

ParsedJson::ParsedJson(ParsedJson && p)
    : bytecapacity(p.bytecapacity),
    depthcapacity(p.depthcapacity),
    tapecapacity(p.tapecapacity),
    stringcapacity(p.stringcapacity),
    current_loc(p.current_loc),
    n_structural_indexes(p.n_structural_indexes),
    structural_indexes(p.structural_indexes),
    tape(p.tape),
    containing_scope_offset(p.containing_scope_offset),
    ret_address(p.ret_address),
    string_buf(p.string_buf),
    current_string_buf_loc(p.current_string_buf_loc),
    isvalid(p.isvalid) {
        p.structural_indexes=nullptr;
        p.tape=nullptr;
        p.containing_scope_offset=nullptr;
        p.ret_address=nullptr;
        p.string_buf=nullptr;
        p.current_string_buf_loc=nullptr;
    }



WARN_UNUSED
bool ParsedJson::allocateCapacity(size_t len, size_t maxdepth) {
    if ((maxdepth == 0) || (len == 0)) {
      std::cerr << "capacities must be non-zero " << std::endl;
      return false;
    }
    if(len > SIMDJSON_MAXSIZE_BYTES) {
      return false;
    }
    if ((len <= bytecapacity) && (depthcapacity < maxdepth)) {
      return true;
    }
    deallocate();
    isvalid = false;
    bytecapacity = 0; // will only set it to len after allocations are a success
    n_structural_indexes = 0;
    uint32_t max_structures = ROUNDUP_N(len, 64) + 2 + 7;
    structural_indexes = new (std::nothrow) uint32_t[max_structures];
    // a pathological input like "[[[[..." would generate len tape elements, so need a capacity of len + 1
    size_t localtapecapacity = ROUNDUP_N(len + 1, 64);
    // a document with only zero-length strings... could have len/3 string
    // and we would need len/3 * 5 bytes on the string buffer 
    size_t localstringcapacity = ROUNDUP_N(5 * len / 3 + 32, 64);
    string_buf = new (std::nothrow) uint8_t[localstringcapacity];
    tape = new (std::nothrow) uint64_t[localtapecapacity];
    containing_scope_offset = new (std::nothrow) uint32_t[maxdepth];
#ifdef SIMDJSON_USE_COMPUTED_GOTO
    ret_address = new (std::nothrow) void *[maxdepth];
#else
    ret_address = new (std::nothrow) char[maxdepth];
#endif
    if ((string_buf == nullptr) || (tape == nullptr) ||
        (containing_scope_offset == nullptr) || (ret_address == nullptr) || (structural_indexes == nullptr)) {
      std::cerr << "Could not allocate memory" << std::endl;
      delete[] ret_address;
      delete[] containing_scope_offset;
      delete[] tape;
      delete[] string_buf;
      delete[] structural_indexes;

      return false;
    }
    /*
    // We do not need to initialize this content for parsing, though we could
    // need to initialize it for safety.
    memset(string_buf, 0 , localstringcapacity); 
    memset(structural_indexes, 0, max_structures * sizeof(uint32_t)); 
    memset(tape, 0, localtapecapacity * sizeof(uint64_t)); 
    */
    bytecapacity = len;
    depthcapacity = maxdepth;
    tapecapacity = localtapecapacity;
    stringcapacity = localstringcapacity;
    return true;
}

bool ParsedJson::isValid() const {
    return isvalid;
}

int ParsedJson::getErrorCode() const {
    return errorcode;
}

std::string ParsedJson::getErrorMsg() const {
  return errorMsg(errorcode);
}

void ParsedJson::deallocate() {
    bytecapacity = 0;
    depthcapacity = 0;
    tapecapacity = 0;
    stringcapacity = 0;
    delete[] ret_address;
    delete[] containing_scope_offset;
    delete[] tape;
    delete[] string_buf;
    delete[] structural_indexes;
    isvalid = false;
}

void ParsedJson::init() {
    current_string_buf_loc = string_buf;
    current_loc = 0;
    isvalid = false;
}

WARN_UNUSED
bool ParsedJson::printjson(std::ostream &os) {
    if(!isvalid) { 
      return false;
    }
    uint32_t string_length;
    size_t tapeidx = 0;
    uint64_t tape_val = tape[tapeidx];
    uint8_t type = (tape_val >> 56);
    size_t howmany = 0;
    if (type == 'r') {
      howmany = tape_val & JSONVALUEMASK;
    } else {
      fprintf(stderr, "Error: no starting root node?");
      return false;
    }
    if (howmany > tapecapacity) {
      fprintf(stderr,
          "We may be exceeding the tape capacity. Is this a valid document?\n");
      return false;
    }
    tapeidx++;
    bool *inobject = new bool[depthcapacity];
    auto *inobjectidx = new size_t[depthcapacity];
    int depth = 1; // only root at level 0
    inobjectidx[depth] = 0;
    inobject[depth] = false;
    for (; tapeidx < howmany; tapeidx++) {
      tape_val = tape[tapeidx];
      uint64_t payload = tape_val & JSONVALUEMASK;
      type = (tape_val >> 56);
      if (!inobject[depth]) {
        if ((inobjectidx[depth] > 0) && (type != ']')) {
          os << ",";
        }
        inobjectidx[depth]++;
      } else { // if (inobject) {
        if ((inobjectidx[depth] > 0) && ((inobjectidx[depth] & 1) == 0) &&
            (type != '}')) {
          os << ",";
        }
        if (((inobjectidx[depth] & 1) == 1)) {
          os << ":";
        }
        inobjectidx[depth]++;
      }
      switch (type) {
      case '"': // we have a string
        os << '"';
        memcpy(&string_length,string_buf + payload, sizeof(uint32_t));
        print_with_escapes((const unsigned char *)(string_buf + payload + sizeof(uint32_t)), string_length); 
        os << '"';
        break;
      case 'l': // we have a long int
        if (tapeidx + 1 >= howmany) {
          delete[] inobject;
          delete[] inobjectidx;
          return false;
        }
        os <<  static_cast<int64_t>(tape[++tapeidx]);
        break;
      case 'd': // we have a double
        if (tapeidx + 1 >= howmany){
          delete[] inobject;
          delete[] inobjectidx;
          return false;
        }
        double answer;
        memcpy(&answer, &tape[++tapeidx], sizeof(answer));
        os << answer;
        break;
      case 'n': // we have a null
        os << "null";
        break;
      case 't': // we have a true
        os << "true";
        break;
      case 'f': // we have a false
        os << "false";
        break;
      case '{': // we have an object
        os << '{';
        depth++;
        inobject[depth] = true;
        inobjectidx[depth] = 0;
        break;
      case '}': // we end an object
        depth--;
        os << '}';
        break;
      case '[': // we start an array
        os << '[';
        depth++;
        inobject[depth] = false;
        inobjectidx[depth] = 0;
        break;
      case ']': // we end an array
        depth--;
        os << ']';
        break;
      case 'r': // we start and end with the root node
        fprintf(stderr, "should we be hitting the root node?\n");
        delete[] inobject;
        delete[] inobjectidx;
        return false;
      default:
        fprintf(stderr, "bug %c\n", type);
        delete[] inobject;
        delete[] inobjectidx;
        return false;
      }
    }
    delete[] inobject;
    delete[] inobjectidx;
    return true;
}

WARN_UNUSED
bool ParsedJson::dump_raw_tape(std::ostream &os) {
    if(!isvalid) { 
      return false;
    }
    uint32_t string_length;
    size_t tapeidx = 0;
    uint64_t tape_val = tape[tapeidx];
    uint8_t type = (tape_val >> 56);
    os << tapeidx << " : " << type;
    tapeidx++;
    size_t howmany = 0;
    if (type == 'r') {
      howmany = tape_val & JSONVALUEMASK;
    } else {
      fprintf(stderr, "Error: no starting root node?");
      return false;
    }
    os << "\t// pointing to " << howmany <<" (right after last node)\n";
    uint64_t payload;
    for (; tapeidx < howmany; tapeidx++) {
      os << tapeidx << " : ";
      tape_val = tape[tapeidx];
      payload = tape_val & JSONVALUEMASK;
      type = (tape_val >> 56);
      switch (type) {
      case '"': // we have a string
        os << "string \"";
        memcpy(&string_length,string_buf + payload, sizeof(uint32_t));
        print_with_escapes((const unsigned char *)(string_buf + payload + sizeof(uint32_t)), string_length);
        os << '"';
        os << '\n';
        break;
      case 'l': // we have a long int
        if (tapeidx + 1 >= howmany) {
          return false;
        }
        os << "integer " << static_cast<int64_t>(tape[++tapeidx]) << "\n";
        break;
      case 'd': // we have a double
        os << "float ";
        if (tapeidx + 1 >= howmany) {
          return false;
        }
        double answer;
        memcpy(&answer, &tape[++tapeidx], sizeof(answer));
        os << answer << '\n';
        break;
      case 'n': // we have a null
        os << "null\n";
        break;
      case 't': // we have a true
        os << "true\n";
        break;
      case 'f': // we have a false
        os << "false\n";
        break;
      case '{': // we have an object
        os << "{\t// pointing to next tape location " << payload << " (first node after the scope) \n";
        break;
      case '}': // we end an object
        os << "}\t// pointing to previous tape location " << payload << " (start of the scope) \n";
        break;
      case '[': // we start an array
        os << "[\t// pointing to next tape location " << payload << " (first node after the scope) \n";
        break;
      case ']': // we end an array
        os << "]\t// pointing to previous tape location " << payload << " (start of the scope) \n";
        break;
      case 'r': // we start and end with the root node
        printf("end of root\n");
        return false;
      default:
        return false;
      }
    }
    tape_val = tape[tapeidx];
    payload = tape_val & JSONVALUEMASK;
    type = (tape_val >> 56);
    os << tapeidx << " : "<< type <<"\t// pointing to " << payload <<" (start root)\n";
    return true;
}
}
/* end file src/parsedjson.cpp */
/* begin file src/parsedjsoniterator.cpp */
#include <iterator>

namespace simdjson {
ParsedJson::iterator::iterator(ParsedJson &pj_) : pj(pj_), depth(0), location(0), tape_length(0), depthindex(nullptr) {
        if(!pj.isValid()) {
            throw InvalidJSON();
        }
        depthindex = new scopeindex_t[pj.depthcapacity];
        // memory allocation would throw
        //if(depthindex == nullptr) { 
        //    return;
        //}
        depthindex[0].start_of_scope = location;
        current_val = pj.tape[location++];
        current_type = (current_val >> 56);
        depthindex[0].scope_type = current_type;
        if (current_type == 'r') {
            tape_length = current_val & JSONVALUEMASK;
            if(location < tape_length) {
                current_val = pj.tape[location];
                current_type = (current_val >> 56);
                depth++;
                depthindex[depth].start_of_scope = location;
                depthindex[depth].scope_type = current_type;
              }
        } else {
            // should never happen
            throw InvalidJSON();
        }
}

ParsedJson::iterator::~iterator() {
      delete[] depthindex;
}

ParsedJson::iterator::iterator(const iterator &o):
    pj(o.pj), depth(o.depth), location(o.location),
    tape_length(0), current_type(o.current_type),
    current_val(o.current_val), depthindex(nullptr) {
    depthindex = new scopeindex_t[pj.depthcapacity];
    // allocation might throw
    memcpy(depthindex, o.depthindex, pj.depthcapacity * sizeof(depthindex[0]));
    tape_length = o.tape_length;
}

ParsedJson::iterator::iterator(iterator &&o):
      pj(o.pj), depth(o.depth), location(o.location),
      tape_length(o.tape_length), current_type(o.current_type),
      current_val(o.current_val), depthindex(o.depthindex) {
        o.depthindex = nullptr;// we take ownership
}

bool ParsedJson::iterator::print(std::ostream &os, bool escape_strings) const {
    if(!isOk()) { 
      return false;
    }
    switch (current_type) {
    case '"': // we have a string
    os << '"';
    if(escape_strings) {
        print_with_escapes(get_string(), os, get_string_length());
    } else {
        // was: os << get_string();, but given that we can include null chars, we have to do something crazier:
        std::copy(get_string(), get_string() + get_string_length(), std::ostream_iterator<char>(os));
    }
    os << '"';
    break;
    case 'l': // we have a long int
    os << get_integer();
    break;
    case 'd':
    os << get_double();
    break;
    case 'n': // we have a null
    os << "null";
    break;
    case 't': // we have a true
    os << "true";
    break;
    case 'f': // we have a false
    os << "false";
    break;
    case '{': // we have an object
    case '}': // we end an object
    case '[': // we start an array
    case ']': // we end an array
    os << static_cast<char>(current_type);
    break;
    default:
    return false;
    }
    return true;
}
}
/* end file src/parsedjsoniterator.cpp */
