
#define COPYRIGHT(eng,v1,v2,v3,v4)	" "eng" Copyright IM Lab, Inc. Ver "QUOTE(v1)"."QUOTE(v2)"."QUOTE(v3)"."QUOTE(v4)" compiled at "__DATE__","__TIME__
#define QUOTE_X(t) #t
#define QUOTE(t) QUOTE_X(t)

#define DEF_VER_RELEASE					1
#define DEF_VER_RELEASE_CANDIDATE		0
#define DEF_VER_BETA					0
#define DEF_VER_ALPHA					0
#define DEF_VER_CONFIG					"RELEASE"

#define DEF_VER_COPYRIGHT		COPYRIGHT(DEF_VER_CONFIG,DEF_VER_RELEASE,DEF_VER_RELEASE_CANDIDATE,DEF_VER_BETA,DEF_VER_ALPHA)
