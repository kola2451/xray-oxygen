#pragma once
// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the XRCDB_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// XRCDB_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#include "../xrCore/xrCore.h"

#ifdef XRCDB_EXPORTS
#	define XRCDB_API __declspec(dllexport)
#else
#	define XRCDB_API __declspec(dllimport)
#endif
#ifdef M_VISUAL
#define ALIGN(a) __declspec(align(a))
#else
#define ALIGN(a)
#endif

// forward declarations
class CFrustum;
class CDB_Model;

#pragma pack(push,8)
namespace CDB
{
    // Triangle for x86
#pragma pack(push, 1)
#if defined(_M_X64)
    class XRCDB_API TRI_DEPRECATED						//*** 16 bytes total (was 32 :)
    {
    public:
        u32				verts[3];		// 3*4 = 12b
        union {
            u32			dummy;				// 4b
            struct {
                u32		material : 14;		// 
                u32		suppress_shadows : 1;	// 
                u32		suppress_wm : 1;		// 
                u32		sector : 16;			// 
            };
        };
    public:
        IC u32			IDvert(u32 ID) { return verts[ID]; }
    };
#endif
#pragma pack (pop)
	// Triangle
	class XRCDB_API TRI						//*** 24 bytes total
	{
	public:
		u32				verts	[3];		// 3*4 = 12b
		union	
		{
			size_t			dummy;				// 4b
			struct 
			{
				size_t		material:14;		// 
				size_t		suppress_shadows:1;	// 
				size_t		suppress_wm:1;		// 
				size_t		sector:16;			// 
#if defined(_M_X64)
                size_t		dumb : 32;
#endif
			};
			struct
			{
				u32 dummy_low;
				u32 dummy_high;
			};
		};

#if defined(_M_X64)
        TRI (TRI_DEPRECATED& oldTri)
        {
            verts[0] = oldTri.verts[0];
            verts[1] = oldTri.verts[1];
            verts[2] = oldTri.verts[2];
            dummy = oldTri.dummy;
            dumb = 0;
        }

        TRI()
        {
            verts[0] = 0;
            verts[1] = 0;
            verts[2] = 0;
            dummy = 0;
        }

        TRI& operator= (const TRI_DEPRECATED& oldTri)
        {
            verts[0] = oldTri.verts[0];
            verts[1] = oldTri.verts[1];
            verts[2] = oldTri.verts[2];
            dummy = oldTri.dummy;
            dumb = 0;
            return *this;
        }
#endif
	public:
		IC u32			IDvert	(u32 ID)		{ return verts[ID];	}
	};

	// Build callback
	typedef		void __stdcall	build_callback	(Fvector* V, int Vcnt, TRI* T, int Tcnt, void* params);

	// Model definition
	class		XRCDB_API		MODEL
	{
		friend class COLLIDER;
		enum
		{
			S_READY				= 0,
			S_INIT				= 1,
			S_BUILD				= 2,
			S_forcedword		= u32(-1)
		};
	private:
		mutable xrCriticalSection lock;
		CDB_Model*				tree;
		volatile u32						status;		// 0=ready, 1=init, 2=building

		// tris
		TRI*					tris;
		int						tris_count;
		Fvector*				verts;
		int						verts_count;
	public:
		MODEL();
		~MODEL();

		IC Fvector*				get_verts		()			{ return verts;		}
		IC const Fvector*		get_verts		()	const	{ return verts;		}
		IC int					get_verts_count	()	const	{ return verts_count;}
		IC const TRI*			get_tris		()	const 	{ return tris;		}
		IC TRI*					get_tris		()			{ return tris;		}
		IC int					get_tris_count	()	const	{ return tris_count;}
		IC void					syncronize		()	const
		{
			if (S_READY!=status)
			{
				Log						("! WARNING: syncronized CDB::query");
				xrCriticalSectionGuard guard(lock);
			}
		}

		static	void build_thread	(void*);
				void build_internal	(Fvector* V, int Vcnt, TRI* T, int Tcnt, void *pCache = nullptr, bool isCacheReader = false, build_callback* bc=nullptr, void* bcp=nullptr, bool rebuildTrisRequired = true);
				void build			(Fvector* V, int Vcnt, TRI* T, int Tcnt, void *pCache = nullptr, bool isCacheReader = false, build_callback* bc=nullptr, void* bcp=nullptr, bool rebuildTrisRequired = true);
		size_t		 memory			();
	};

	// Collider result
	struct XRCDB_API RESULT
	{
		Fvector			verts	[3];
		union
		{
			size_t		dummy;				// 8b
			struct {
				size_t		material:14;		// 
				size_t		suppress_shadows:1;	// 
				size_t		suppress_wm:1;		// 
				size_t		sector:16;			// 
#if defined(_M_X64)
				u64			dumb : 32;
#endif
			};

			struct 
			{
				u32 dummy_low;
				u32 dummy_high;
			};
		};
		int				id;
		float			range;
		float			u,v;
	};

	// Collider Options
	enum {
		OPT_CULL		= (1<<0),
		OPT_ONLYFIRST	= (1<<1),
		OPT_ONLYNEAREST	= (1<<2),
		OPT_FULL_TEST   = (1<<3)		// for box & frustum queries - enable class III test(s)
	};

	// Collider itself
	class XRCDB_API COLLIDER
	{
		// Ray data and methods
		u32				ray_mode;
		u32				box_mode;
		u32				frustum_mode;

		// Result management
		xr_vector<RESULT>	rd;
	public:
		COLLIDER		();
		~COLLIDER		();

		ICF void		ray_options		(u32 f)	{	ray_mode = f;		}
		void			ray_query		(const MODEL *m_def, const Fvector& r_start,  const Fvector& r_dir, float r_range = 10000.f);

		ICF void		box_options		(u32 f)	{	box_mode = f;		}
		void			box_query		(const MODEL *m_def, const Fvector& b_center, const Fvector& b_dim);

		ICF void		frustum_options	(u32 f)	{	frustum_mode = f;	}
		void			frustum_query	(const MODEL *m_def, const CFrustum& F);

#ifndef PVS_STUDIO
		ICF RESULT*		r_begin			()	{	return &*rd.begin(); };
		ICF RESULT*		r_end			()	{	return &*rd.end(); };
		RESULT&			r_add			()	;
		void			r_free			()	;
		ICF size_t		r_count			()	{	return rd.size(); };
		ICF void		r_clear			()	{	rd.clear(); };
		ICF void		r_clear_compact	()	{	rd.clear(); };
#endif
	};

	//
	class XRCDB_API Collector
	{
		xr_vector<Fvector>	verts;
		xr_vector<TRI>		faces;

		u32				VPack				( const Fvector& V, float eps);
	public:
		void			add_face			( const Fvector& v0, const Fvector& v1, const Fvector& v2, u16 material, u16 sector	);
		void			add_face_D			( const Fvector& v0, const Fvector& v1, const Fvector& v2, size_t dummy );
		void			add_face_packed		( const Fvector& v0, const Fvector& v1, const Fvector& v2, u16 material, u16 sector, float eps = EPS );
		void			add_face_packed_D	( const Fvector& v0, const Fvector& v1, const Fvector& v2, size_t dummy, float eps = EPS );
        void			remove_duplicate_T	( );
		void			calc_adjacency		( xr_vector<u32>& dest		);

		Fvector*		getV			()	{ return &*verts.begin();		}
		size_t			getVS			() 	{ return verts.size();			}
		TRI*			getT			()	{ return &*faces.begin();		}
		size_t			getTS			()	{ return faces.size();			}
		void			clear			()	{ verts.clear(); faces.clear();	}
	};

	struct non_copyable // C++11 Style: on :))
	{
						non_copyable	() {}
	private:
						non_copyable	(const non_copyable &) = delete;
						non_copyable	&operator=		(const non_copyable &) = delete;
	};

#pragma warning(push)
#pragma warning(disable:4275)
	const u32 clpMX = 24, clpMY=16, clpMZ=24;
	class XRCDB_API CollectorPacked : public non_copyable {
		typedef xr_vector<u32>		DWORDList;
		typedef DWORDList::iterator	DWORDIt;
	
	private:
		xr_vector<Fvector>			verts;
		xr_vector<TRI>		faces;
		xr_vector<u32>		flags;
		Fvector				VMmin, VMscale;
		DWORDList			VM		[clpMX+1][clpMY+1][clpMZ+1];
		Fvector				VMeps;

		u32					VPack		( const Fvector& V);
	public:
		CollectorPacked	(const Fbox &bb, int apx_vertices=5000, int apx_faces=5000);

		void				add_face	( const Fvector& v0, const Fvector& v1, const Fvector& v2, u16 material, u16 sector, u32 flags );
		void				add_face_D	( const Fvector& v0, const Fvector& v1, const Fvector& v2, size_t dummy , u32 flags );

		xr_vector<Fvector>& getV_Vec()			{ return verts;				}
		Fvector*			getV()				{ return &*verts.begin();	}
		size_t				getVS()				{ return verts.size();		}
		TRI*				getT()				{ return &*faces.begin();	}
		u32					getfFlags(u32 index){ return flags[index];		}	
IC		TRI&				getT(u32 index)		{ return faces[index];		}
		size_t				getTS()				{ return faces.size();		}
		void				clear();
	};
#pragma warning(pop)
};

#pragma pack(pop)

