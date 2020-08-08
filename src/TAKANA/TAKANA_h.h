

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Tue Jan 19 12:14:07 2038
 */
/* Compiler settings for TAKANA.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0622 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */


#ifndef __TAKANA_h_h__
#define __TAKANA_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ITAKANA_FWD_DEFINED__
#define __ITAKANA_FWD_DEFINED__
typedef interface ITAKANA ITAKANA;

#endif 	/* __ITAKANA_FWD_DEFINED__ */


#ifndef __CTAKANADoc_FWD_DEFINED__
#define __CTAKANADoc_FWD_DEFINED__

#ifdef __cplusplus
typedef class CTAKANADoc CTAKANADoc;
#else
typedef struct CTAKANADoc CTAKANADoc;
#endif /* __cplusplus */

#endif 	/* __CTAKANADoc_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __TAKANA_LIBRARY_DEFINED__
#define __TAKANA_LIBRARY_DEFINED__

/* library TAKANA */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_TAKANA;

#ifndef __ITAKANA_DISPINTERFACE_DEFINED__
#define __ITAKANA_DISPINTERFACE_DEFINED__

/* dispinterface ITAKANA */
/* [uuid] */ 


EXTERN_C const IID DIID_ITAKANA;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("012ecf19-e7da-4d0c-b840-5b95fbcac204")
    ITAKANA : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct ITAKANAVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAKANA * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAKANA * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAKANA * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITAKANA * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITAKANA * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITAKANA * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITAKANA * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } ITAKANAVtbl;

    interface ITAKANA
    {
        CONST_VTBL struct ITAKANAVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAKANA_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITAKANA_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITAKANA_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ITAKANA_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ITAKANA_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ITAKANA_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ITAKANA_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __ITAKANA_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_CTAKANADoc;

#ifdef __cplusplus

class DECLSPEC_UUID("f0a3386e-6ab4-432f-89ad-68be1ec8a3d9")
CTAKANADoc;
#endif
#endif /* __TAKANA_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


