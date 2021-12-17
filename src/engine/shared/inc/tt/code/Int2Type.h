#if !defined(INC_TT_CODE_INT2TYPE_H)
#define INC_TT_CODE_INT2TYPE_H


namespace tt {
namespace code {


template <int V>
class Int2Type
{
public:
	enum {value = V};
};


// End namespace
}
}


#endif // !defined(INC_TT_CODE_INT2TYPE_H)
