#if !defined(INC_TT_PIRACTY_OBFUSCATE_H)
#define INC_TT_PIRACTY_OBFUSCATE_H


#define OBFUSCATE_DATA_1(obfuscator, type, a)                                         { _OBFUSCATE_DATA_1(obfuscator, type, a) }                                      
#define OBFUSCATE_DATA_2(obfuscator, type, a,b)                                       { _OBFUSCATE_DATA_2(obfuscator, type, a,b) }                                    
#define OBFUSCATE_DATA_3(obfuscator, type, a,b,c)                                     { _OBFUSCATE_DATA_3(obfuscator, type, a,b,c) }                                  
#define OBFUSCATE_DATA_4(obfuscator, type, a,b,c,d)                                   { _OBFUSCATE_DATA_4(obfuscator, type, a,b,c,d) }                                
#define OBFUSCATE_DATA_5(obfuscator, type, a,b,c,d,e)                                 { _OBFUSCATE_DATA_5(obfuscator, type, a,b,c,d,e) }                              
#define OBFUSCATE_DATA_6(obfuscator, type, a,b,c,d,e,f)                               { _OBFUSCATE_DATA_6(obfuscator, type, a,b,c,d,e,f) }                            
#define OBFUSCATE_DATA_7(obfuscator, type, a,b,c,d,e,f,g)                             { _OBFUSCATE_DATA_7(obfuscator, type, a,b,c,d,e,f,g) }                          
#define OBFUSCATE_DATA_8(obfuscator, type, a,b,c,d,e,f,g,h)                           { _OBFUSCATE_DATA_8(obfuscator, type, a,b,c,d,e,f,g,h) }                        
#define OBFUSCATE_DATA_9(obfuscator, type, a,b,c,d,e,f,g,h,i)                         { _OBFUSCATE_DATA_9(obfuscator, type, a,b,c,d,e,f,g,h,i) }                      
#define OBFUSCATE_DATA_10(obfuscator, type, a,b,c,d,e,f,g,h,i,j)                      { _OBFUSCATE_DATA_10(obfuscator, type, a,b,c,d,e,f,g,h,i,j) }                   
#define OBFUSCATE_DATA_11(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k)                    { _OBFUSCATE_DATA_11(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k) }                 
#define OBFUSCATE_DATA_12(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l)                  { _OBFUSCATE_DATA_12(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l) }               
#define OBFUSCATE_DATA_13(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m)                { _OBFUSCATE_DATA_13(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m) }             
#define OBFUSCATE_DATA_14(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n)              { _OBFUSCATE_DATA_14(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n) }           
#define OBFUSCATE_DATA_15(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o)            { _OBFUSCATE_DATA_15(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) }         
#define OBFUSCATE_DATA_16(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)          { _OBFUSCATE_DATA_16(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) }       
#define OBFUSCATE_DATA_17(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q)        { _OBFUSCATE_DATA_17(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) }     
#define OBFUSCATE_DATA_18(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r)      { _OBFUSCATE_DATA_18(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) }   
#define OBFUSCATE_DATA_19(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s)    { _OBFUSCATE_DATA_19(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s) } 
#define OBFUSCATE_DATA_20(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t)  { _OBFUSCATE_DATA_20(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t) }

#define _OBFUSCATE_DATA_1(obfuscator, type, a)                                                                                                                      type(obfuscator(a))
#define _OBFUSCATE_DATA_2(obfuscator, type, a,b)                                       _OBFUSCATE_DATA_1(obfuscator, type, a),                                      type(obfuscator(b))
#define _OBFUSCATE_DATA_3(obfuscator, type, a,b,c)                                     _OBFUSCATE_DATA_2(obfuscator, type, a,b),                                    type(obfuscator(c))
#define _OBFUSCATE_DATA_4(obfuscator, type, a,b,c,d)                                   _OBFUSCATE_DATA_3(obfuscator, type, a,b,c),                                  type(obfuscator(d))
#define _OBFUSCATE_DATA_5(obfuscator, type, a,b,c,d,e)                                 _OBFUSCATE_DATA_4(obfuscator, type, a,b,c,d),                                type(obfuscator(e))
#define _OBFUSCATE_DATA_6(obfuscator, type, a,b,c,d,e,f)                               _OBFUSCATE_DATA_5(obfuscator, type, a,b,c,d,e),                              type(obfuscator(f))
#define _OBFUSCATE_DATA_7(obfuscator, type, a,b,c,d,e,f,g)                             _OBFUSCATE_DATA_6(obfuscator, type, a,b,c,d,e,f),                            type(obfuscator(g))
#define _OBFUSCATE_DATA_8(obfuscator, type, a,b,c,d,e,f,g,h)                           _OBFUSCATE_DATA_7(obfuscator, type, a,b,c,d,e,f,g),                          type(obfuscator(h))
#define _OBFUSCATE_DATA_9(obfuscator, type, a,b,c,d,e,f,g,h,i)                         _OBFUSCATE_DATA_8(obfuscator, type, a,b,c,d,e,f,g,h),                        type(obfuscator(i))
#define _OBFUSCATE_DATA_10(obfuscator, type, a,b,c,d,e,f,g,h,i,j)                      _OBFUSCATE_DATA_9(obfuscator, type, a,b,c,d,e,f,g,h,i),                      type(obfuscator(j))
#define _OBFUSCATE_DATA_11(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k)                    _OBFUSCATE_DATA_10(obfuscator, type, a,b,c,d,e,f,g,h,i,j),                   type(obfuscator(k))
#define _OBFUSCATE_DATA_12(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l)                  _OBFUSCATE_DATA_11(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k),                 type(obfuscator(l))
#define _OBFUSCATE_DATA_13(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m)                _OBFUSCATE_DATA_12(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l),               type(obfuscator(m))
#define _OBFUSCATE_DATA_14(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n)              _OBFUSCATE_DATA_13(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m),             type(obfuscator(n))
#define _OBFUSCATE_DATA_15(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o)            _OBFUSCATE_DATA_14(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n),           type(obfuscator(o))
#define _OBFUSCATE_DATA_16(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)          _OBFUSCATE_DATA_15(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o),         type(obfuscator(p))
#define _OBFUSCATE_DATA_17(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q)        _OBFUSCATE_DATA_16(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p),       type(obfuscator(q))
#define _OBFUSCATE_DATA_18(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r)      _OBFUSCATE_DATA_17(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q),     type(obfuscator(r))
#define _OBFUSCATE_DATA_19(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s)    _OBFUSCATE_DATA_18(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r),   type(obfuscator(s))
#define _OBFUSCATE_DATA_20(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t)  _OBFUSCATE_DATA_19(obfuscator, type, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s), type(obfuscator(t))


#endif // !defined(INC_TT_PIRACTY_OBFUSCATE_H)
