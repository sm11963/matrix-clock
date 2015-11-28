/*  GIMP header image file format (RGB): /Users/smiller/Desktop/Icon-Small.h  */

static unsigned int width = 29;
static unsigned int height = 29;

/*  Call this macro repeatedly.  After each use, the pixel data can be extracted  */

#define HEADER_PIXEL(data,pixel) {\
pixel[0] = (((data[0] - 33) << 2) | ((data[1] - 33) >> 4)); \
pixel[1] = ((((data[1] - 33) & 0xF) << 4) | ((data[2] - 33) >> 2)); \
pixel[2] = ((((data[2] - 33) & 0x3) << 6) | ((data[3] - 33))); \
data += 4; \
}
static char *header_data =
	"7K@=7K@=7K@=7;<=7K@=7;<=7;<=7K@=7;<=7K@=7K@=7K@=7K@=7K@=7K@=7K@="
	"7K@=7K@=7K@=7K@=7K@=7K@=7K@=7K@=7K@=7K@=7K@=7K@=7K@=7+8;6[4;6[4;"
	"7+8;6[4;7+8;7+8;7+8;7+8;7+8;7+8;7+8;7+8;7+8;7+8;6[4;7+8;6[4;6[4;"
	"6[4:6[4;6[4:6[4:7+8;6[4:7+8;7+8;6[4:7+8;6;,86K096K096;,96K096;,9"
	"6;,96;,86;,96;,86;,86;,86;,86;,86;,86;096;,86;096;096;096;096;09"
	"6;096K096;096K096K096K096K095[$75[(75[(7````]`D]\\@8[_@T^_0P^\\@8["
	"\\@8[\\@8[\\@8[\\@8[\\@8[\\@8[\\@8[\\@8[\\@8[\\@8[\\@8[\\@8[_0P^_@T^\\@8[]`D]"
	"````5[(75[$65[(75:\\55*\\45*\\4````C<XB5*\\4S?(RP^PP5*\\45:\\55:\\55*\\4"
	"5:\\55*\\45*\\45:\\55*\\45:\\55:\\55:`55:\\5P^PPS?(R5:\\5CL\\C````5:\\55*\\4"
	"5:\\54JT24ZX34ZX3````ZP(YWOLV]`D]PNLO4JT24JT34JT34ZX34JT34ZX34ZX3"
	"4JT24ZX34JT2<[\\:4ZT34JT2PNLO]`D]WOLVZP(Y````4JT24ZT34JT24*L14*L0"
	"4*L0````E],C8K45T?0SP>LN4*L14*L04*L04*L04*L04*L04*L04*L15*T1P.HN"
	"`0X_?L4<4*L1P>LNT?0R8K45E],C````4*L14*L04*L13:@.3:@.3:@.````T/,R"
	"N.4K[0,YP.HM3JD.3:@.3:@.3JD.3:@.3JD.3JD.7+$2V/<T````^`L^A,<=3JD/"
	"P.HM[0(YN.4KT?,R````3JD.3:@.3JD.2Z8,3*<,3*<,````KM\\GA,<<W/HUP.DM"
	"3*<-3:<,K=\\G><$93*<,2Z8,:K<4Z`(Y````[`0Z;KH53*<,2Z8,P.DMW/DTA,<<"
	"KM\\G````3*<-2Z8,3*<-2:0*2:0*2:0*````M>,IC\\T>W_LUO^@L2:0*F],A````"
	"]@@\\8[,1?L,9^0H]````W/DT7*\\02J4+2:0*2J4+O^@LW_LUC\\T>M>,I````2:0*"
	"2:0*2:0*1Z((1Z,(1Z,(````QNPMJ-HDY_`WON<K1Z((3*4)VO@S````\\04Z`0X_"
	"````Q.LM3:4)1Z((1Z((1Z((1Z((ON<KY_`WJ-HDQNPM````1Z((1Z((1Z((19`&"
	"1)`&1)`&````F-$?9;,0T_,QO><J19`&1)`%5:H*[@,Y````_@P^I=DC1)`%19`&"
	"1)`%1)`%1)`&1)`%O><JT_,Q9+,0F=(?````1)`&19`&1)`&0IX$0IX$0IX$````"
	"W_HTSN`O\\`8ZO.8J0IX#0IX$0IX$9K,/Z?`X;;<10YX$0IX$0YX$0IX$0IX$0YX$"
	"0IX$O.8J\\`8[SN`OW_HT````0IX$0IX#0IX$/YL!0)P\"0)P\"````@,(60)P\"Q^PM"
	"N^4I0)P\"0)P\"0)P\"0)L!0YT\"0)L!0)L!/YL!0)L!/YL!/YL!0)P!/YL!N^4IQ^PL"
	"/YL!@,(5````/YL!0)P\"/YL!/9C_/IG`/IG`````]@@[[P,Y_0P]_0L][P,Y[P,Y"
	"[P,Y[P,Y[P,Y[P,Y[P,Y[P,Y[P,Y[P,Y[P,Y[P,Y[P,Y_0L]_0P][P,Y]@@[````"
	"/IG`/IG`/IG`.Y;].Y;].Y;]````?+X3.Y?^Q>LKNN,G.Y?^.Y;].Y;]/)?^.Y;]"
	"/)?^/)?^.Y;]/)?^.Y;].Y;].Y;].Y;]NN,GQ>HK.Y;]?+X3````.Y;].Y;].Y;]"
	".97\\.93\\.93\\````XOPUTO(O]0<[N>,F.)3[.93[.93[.93[.93[.93[.93[.93\\"
	".93[/Y?^@;`4.93[.93\\N>,F]0<[TO(OXOPT````.97\\.93[.97\\-Y+Y-I+Y-I+Y"
	"````C,<74Z0\"S>\\MN.(F-Y+Y-Y+Z-Y+Z-Y+Z-Y+Z-Y+Z-Y+Z-I+Y2IW`UO4P````"
	"A<,4-I+YN.(FS>\\M4Z0\"C,<7````-Y+Y-I'Y-Y+Y-(_W-9#W-9#W````Q>HJIM<?"
	"YOXUM^$E-)#W-)#W-)#W-(_W-)#W-(_W-(_W6J@$Z`$W````Z?`W6*<#-)#WM^$E"
	"YOXUIM<?Q>HJ````-9#W-9#W-9#W,H[U,H[U,H[U````I]<?>KL.V?8QMM`D,H[U"
	"/97YQ^LJ@\\$2,H[U,H[U<K8,^@H\\````U/(O1)C[,HWU,HWUMM`DV?8Q>KL.I]8?"
	"````,H[U,HWU,H[U,(OS+XOR+XOR````I=8=>+D-V/4PMM`C,(OSB<03````_PT^"
	"9*T&D<D6`@X_````MM`C-8[U,(OS,(SS,(OSMM`CV/4P>+D-IM8>````,(OS+XOS"
	",(OS+8GQ+HKQ+HKQ````P^@HH]0<Y?TUM-\\C+8GP+8GQN>$D````_PT^`````P\\_"
	"E,H6+HKQ+8GP+8GP+HKQ+8GPM=\\CY?TUH]0<P^@H````+8GP+HKQ+8GP*X?O*X?O"
	"*X?O````AL$12)GZRNPJM-XB*X?O*X?N,8OPU?,N````[@(X<;0)*X?O*X?O*X?O"
	"*X?O*X?O*X?OM-XBRNPJ2)GZAL$1````*X?O*X?N*X?O*(7L*(7L*(7L````W_DR"
	"SNXL\\`8ZL]TA*87M*(3L*(3L/9'TP.4F0)/U*87M*87M*87M*87M*87M*(7L*87M"
	"L]TA\\`8ZSNXLW_DR````*87M*(7L*87M)H/J)X/K)X/K````;[(')X/KO^4FLMPA"
	")X/K)X/K)X/K)H/J)X/K)H/J)H/J)H+J)H/J)H+J)H+J)X/K)H+JLMPAO^4F)X/K"
	";[('````)X/K)H+J)X/K)(#H)('H)('H````^@D\\]08Z_PT^_@P]]08Z]08Z]08Z"
	"]08Z]08Z]08Z]08Z]08Z]08Z]08Z]08Z]08Z]08Z_@P]_PT^]08Z^@D\\````)('H"
	")(#H)('H(G[F(G_G(G_G(G[F(G_G(G[F(G[F(G_F(G[F(G_F(G_F(G[F(G_F(G[F"
	"(G[F(G[F(G[F(G[F(G[F(G_G(G[F(G_G(G_G(G[F(G_G(G[F(G[F(G_G(G[F('WD"
	"'WSD'WSD('WE'WSD('WE('WE'WSD('WE'WSD'WSD('WD'WSD('WD('WD'WSD('WD"
	"'WSD'WSD'WSD'WSD'WSD'WSD'WSD'WSD'WSD'WSD('WE'WSD'7KB'7KB'7KB'7KB"
	"'7KB'7KB'7KB'7KB'7KB'7KB'7KB'GOC'7KB'GOC'GOC'7KB'GOC'7KB'7KB'GKB"
	"'7KB'GKB'GKB'7KB'GKB'7KB'7KB'7KB'7KB";
