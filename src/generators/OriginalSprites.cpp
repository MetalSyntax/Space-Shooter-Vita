#include "generators/OriginalSprites.hpp"

namespace OriginalSprites {

const EncodedSprite ship = {
    { // palette
        0xf3ff00, 0xe0e0e0, 0xe9f054, 0xc0c585, 0xbdbde4, 0xd7cf2a, 0xaaaad9, 0xabaf62, 0x9e9ede, 0x959693, 0x8e9469, 0xc18e77, 0x8e8eaa, 0x878983, 0x8080ed, 0x808080, 0xc4604d, 0x7676d4, 0x6e6e92, 0x797979, 0x6161bc, 0x686b92, 0x5757ff, 0x5a5bc1, 0x4e4ece, 0x4949b6, 0x4c4daf, 0xd62d20, 0xe00001, 0xdf0002, 0xbf0000, 0xad0001, 0x980000, 0x7e006b, 0x830000, 0x3c3de0, 0x3737c9, 0x3434cd, 0x2828e7, 0x2020df, 0x1b1beb, 0x1313f2, 0x1111ec, 0x0908f7, 0x280bc5, 0x600000, 0x0000ff
    },
    { // rows
        ".........................888..................................",
        ".........................888..................................",
        ".........................888..................................",
        ".........................888888...............................",
        ".........................888888...............................",
        ".........................888888...............................",
        ".........................8888888888...........................",
        ".........................8888888888...........................",
        ".........................8888888888...........................",
        "......................kkhCCC6448CCC88.........................",
        "......................kkkFFF4119FFC888........................",
        "......................kkkFFF4119FFC888........................",
        "................kkkkkkkkhPPPEEEDFFC888........................",
        "...............kkkkkkkkkkkkkkkkLFFC888........................",
        "...............kkkkkkkkkkkkkkkkLFFC888........................",
        "...jjjYWWUSSXkkggghkkkggggggeggQIIK88Hkk......................",
        "...jjjYWWUSSXkkgJJakkkFFFFFFFFFdkkaFFPkkk.....................",
        "...jjjYWWUSSXkkgJJakkkFFFFFFFFFdkkaFFPkkk.....................",
        "WWWjjjWWWUTTXddPAAakkhAAAIIIDAAaddPIIPggekkkkkgWWW............",
        "WWWWWWUTTRBB500000Lkkk000kkk700000AkkaFFIkkkkkkWWW............",
        "WWWWWWUTTRBB500000Lkkk000kkk700000AkkaFFIkkkkkkWWW............",
        "WWWYYYUUURGGG77555Qkkk000kkk700000AkkbFFLkkhkkkWWW............",
        "...jjjYWWUSSXkkgJJakkk111kkk611111EkkkkkgFFA000kkfMMM411......",
        "...jjjYWWUSSXkkgJJakkk111kkk611111EkkkkkgFFA000kkfMMM411......",
        "...jjjYWWUSSXkkgJJbkkk111kkk811111EkkhkkgFFF000kkfMMM411......",
        "...............kkkkkkkFFFFFFFFFdkkaFFFFFIkkkkkk888888ckkMMMM..",
        "...............kkkkkkkFFFFFFFFFdkkaFFFFFIkkkkkk888888ckkMMMM..",
        "...............kkkkkkkFFFFFFFFFdkkaFFFFFIkkkkkk888888ckkMMMM..",
        "..................QLLLdddddddddA775552446666668557cccE66fffhkk",
        "..................FFFFkkkkkkkkk7000002111111111007kkk611kkkkkk",
        "..................FFFFkkkkkkkkk7000002111111111007kkk611kkkkkk",
        "...............kkkgdddLLLLLLLLLPaaL77A99CZZcZZZ336EEEMZZMMMc..",
        "...............kkkkkkkFFFFFFFFFdkkaFFFFFIkkkkkk888888ckkMMMM..",
        "...............kkkkkkkFFFFFFFFFdkkaFFFFFIkkkkkk888888ckkMMMM..",
        "...jjjYWWUSSXkkgaabkkh777PPPA77DLLLPPaPPPaaPLLLMMMEEEMMMMMM...",
        "...jjjYWWUSSXkkgJJLkkk000kkk700000AkkkkkgFFA000kkfMMM411......",
        "...jjjYWWUSSXkkgJJLkkk000kkk700000AkkkkkgFFA000kkfMMM411......",
        "WWWYYYVVVRRRGLLF77Akkh222kkk922222CkkeaaaPPPAAAiiiMMM411......",
        "WWWWWWUTTRBB5000005kkk111kkk611111EkkaFFIkkkkkkWWW............",
        "WWWWWWUTTRBB5000005kkk111kkk611111EkkaFFIkkkkkkWWW............",
        "WWWYYYVVVRRRGLLF77Akkh999PPPC99HMMOPPaaaakkkkkhWWW............",
        "...jjjYWWUSSXkkgJJLkkkFFFFFFFFFdkkaFFPkkk.....................",
        "...jjjYWWUSSXkkgJJLkkkFFFFFFFFFdkkaFFPkkk.....................",
        "...jjjYWWUSSXkkeLLQkkkIIIIIILIIdggPDDNkkk.....................",
        "...............kkkkkkkkkkkkkkkkLFFC888........................",
        "...............kkkkkkkkkkkkkkkkLFFC888........................",
        "...............kkkkkkkkkhdddZZZIFFC888........................",
        "......................kkkFFF4119FFC888........................",
        "......................kkkFFF4119FFC888........................",
        "......................kkk999411C99C888........................",
        ".........................8888888888...........................",
        ".........................8888888888...........................",
        ".........................8888888888...........................",
        ".........................888888...............................",
        ".........................888888...............................",
        ".........................888888...............................",
        ".........................888..................................",
        ".........................888..................................",
        ".........................888..................................",
    }
};

const EncodedSprite enemy = {
    { // palette
        0xe1ebf2, 0xe0eaf2, 0xdfe9f1, 0xdfe9f0, 0xdee8f0, 0xdde8ef, 0xdce8ef, 0xdce7ef, 0xdae5ee, 0xd8e4ed, 0xd8e3ed, 0xd7e2ec, 0xd6e1eb, 0xd5e1eb, 0xd5e0ea, 0xd4e0ea, 0xcedbe7, 0xcedbe6, 0xcddae6, 0xccd9e5, 0xcbd8e4, 0xcad7e4, 0xc9d7e3, 0xc7d5e3, 0xc6d4e2, 0xc5d3e1, 0xbdccdc, 0xbcccdc, 0xbccbdc, 0xbacada, 0xb8c8d9, 0xb7c7d8, 0xb6c6d8, 0xb6c6d7, 0xb5c6d7, 0xb3c4d6, 0xb0c1d4
    },
    { // rows
        "..................0000000000000...................",
        "..................0000000000000...................",
        "..................0000000000000...................",
        "...............0000000000000000...................",
        "...............0000000000000000...................",
        "...............0000000000000000...................",
        ".........00000000000000000000000000000............",
        ".........00000000000000000000000000000............",
        ".........00000000000000000000000000000............",
        "......0000000000006JJI001JJJA000000000000.........",
        "......0000000000008aaa000aaaE000000000000.........",
        "......0000000000008aaa000aaaE000000000000.........",
        "......0000000000009PPO004PPP8000000000000000......",
        "......00000000000000000000000000000000000000......",
        "......00000000000000000000000000000000000000......",
        "......0000000000000000000000000000788F88D88Laaaaaa",
        "......0000000000000000000000000000Jaaaaaaaaaaaaaaa",
        "......0000000000000000000000000000Jaaaaaaaaaaaaaaa",
        "0000000000000000000000000000000000Haaaaaaaaaaaaaaa",
        "0000000000000000000000000000000000Jaaaaaaaaaaaaaaa",
        "0000000000000000000000000000000000Jaaaaaaaaaaaaaaa",
        "0000000000003002003000000000000000Jaaaaaaaaaaaaaaa",
        "000000000000VaaaaaV000000000000000Jaaaaaaaaaaaaaaa",
        "000000000000VaaaaaV000000000000000Jaaaaaaaaaaaaaaa",
        "000000000000UaaaaaW000000000000000Gaaaaaaaaaaaaaaa",
        "...000000000JaaaaaV000000000000000Jaaaaaaaaaaaaaaa",
        "...000000000JaaaaaV000000000000000Jaaaaaaaaaaaaaaa",
        "...000000000JaaaaaV000000000000000Jaaaaaaaaaaaaaaa",
        "...0000000005EEDEEB000000000NRRSRRVaaaaaaaaaaaaaaa",
        "...0000000000000000000000000Raaaaaaaaaaaaaaaaaaaaa",
        "...0000000000000000000000000Raaaaaaaaaaaaaaaaaaaaa",
        "...RRRC000000000000000000RRRZaaaaaaaaaaaaaaa......",
        "...aaaE000000000000000000aaaaaaaaaaaaaaaaaaa......",
        "...aaaE000000000000000000aaaaaaaaaaaaaaaaaaa......",
        "...aaaXVVQVVJ000000003VVYaaaaaaaaaaaaaaaaaaa......",
        "......aaaaaaP000000000aaaaaaaaaaaaaaaaaaaaaa......",
        "......aaaaaaP000000000aaaaaaaaaaaaaaaaaaaaaa......",
        "......aaaaaaTJJMJJMJJKaaaaaaaaaaaaaaaaaaaaa.......",
        "......aaaaaaaaaaaaaaaaaaaaaaaaaaaaa..aaaa.........",
        "......aaaaaaaaaaaaaaaaaaaaaaaaaaaaa..aaaa.........",
        "......aaaaaaaaaaaaaaaaaaaaaaaaaaaa...aaaa.........",
        "......aaaaaaaaaaaaaaaaaaaaaaaaa...................",
        "......aaaaaaaaaaaaaaaaaaaaaaaaa...................",
        "......aaaaaaaaaaaaaaaaaaaaaaaaa...................",
        ".........aaaaaaaaaaaaaaaaaaa......................",
        ".........aaaaaaaaaaaaaaaaaaa......................",
        ".........aaaaaaaaaaaaaaaaaaa......................",
        "..................aaaaaaa.........................",
        "..................aaaaaaa.........................",
        "..................aaaaaaa.........................",
    }
};

const EncodedSprite enemyShip = {
    { // palette
        0x737372, 0x717271, 0x8c5857, 0x64605f, 0x5f5757, 0x535453, 0x494748, 0x393838, 0x363736, 0x353938, 0x363636, 0x353636, 0xe0211f, 0xdb1318, 0xda1218, 0xc30f14, 0xbc0e13, 0x99171a, 0x522e2e, 0x343333, 0x303030, 0x4a2323, 0x4b1518, 0x98060b, 0x95060c, 0x94060c, 0x84060a, 0x770609, 0x620406, 0x600406, 0x5f0405, 0x580507, 0x4b0709, 0x47080a, 0x470709, 0x470609, 0x470608, 0x460709, 0x460609, 0x460608, 0x460507, 0x440508, 0x360405
    },
    { // rows
        "................Mb...................bf................",
        "................RQR.................RQR................",
        "..............fbGENc...............YNEGcf..............",
        "..............fYGCHb...............YHCGcf..............",
        "..............fYCCHb...............YHCCYf..............",
        "..............fY2CHb...............YHC2Zf..............",
        "..............fb305Y.....b..IYI....b503cf..............",
        "..............fY315Z.....c..IbI....Y513Zf..............",
        "..............fYJ8LY.....TVcVTI....YLAJZf..............",
        "..............LLJALM....ISVZVSI....LLAJLL..............",
        "................fZ.....cRORZRORbI....cf................",
        "................gc.....cROQSQPRcI....bf................",
        "................fc.....cROPOOORcI....Yf................",
        "................gb....LZROOOOORZI....Yf................",
        "................fb..LZROGEDDDDGPRZW..bf................",
        "................fc..LcROGDDDDDGORcX..cf................",
        "..............fbbcbbbbRPRcccbcRORcbbbcbbf..............",
        "..............fbXXXcccQNRcZWWWRNQcbcXXXdf..............",
        "........J777..fcJALcZcGENcVTSTNEGcccL8Jbf..777J........",
        "........ABBB..McJ8LcZZGDNcVTSSNEGcccL8JcM..7777........",
        "......K831115YYcJ8LZVTGENcVSSSNEGTVZL8JZZY600038K......",
        "......KJ30004cbbJ8LcVUGDNZVSSSNDGUVZL8JbbY50003JK......",
        "....J8310158Lc315ALcQPFENZVSTTNEFORZL8513YL8500038K....",
        "....KB300059Lc3158LZQOFENcVSTSNDFORZL8503cL850003BK....",
        "..KB30005ALSSS3058LZQOFENZZcZcNDFORZL8503SSTLA500048J..",
        "..KJ300058LSTS3158LcQOFENfffffNDFORcL8503TSTL850004JK..",
        "K830005ALTTSSS315cVTQOFDDEDEEEEDFOQSVc603STTSSL850003AK",
        "K8300059LUTSST315ZVUQOFDDDDDDDDDFOQUVZ613TTTTUL9500038K",
        "K83058LUQORSSS315ZQOOOGEGOOPPPGDFOOOQZ613TTTROQTLA5038K",
        "K83058LUQORUTT305cQOOOFEGOPPPPGEFOOOQZ603SUUROQUL85038K",
        "KA305AHPOOOPRT315cQOOOFDGOPPOPGEFOONQZ613TROOOOPHA5038K",
        "KA305AHOOOOORT316cQOOPGEGPOPPPGEFOOOQZ613TRPOOOPHB5138K",
        "K8305AHOOOPONNIJLcQOOOGEGOOPPOGDFOOOQZLJINNOOOPPHA5038K",
        "K8305AHOOOOOOPIALcQOOOGEGOPOOPGDFOOOQZLAIOOOPOPOH8503AK",
        "K8305AHPOOOORUWeaYccQOGEGPOPPOGEFORcbcdeWUROPOOOHA5038K",
        "KA3058HOOOOORUWZYYYcQOFEGOOOOOGEFORYYbYZWTROOOPPH85038K",
        "KA3058HOOORUWbcY..XYVUQPGDDDDDGPQUVZL..ZccWUROOPHA503AK",
        "KA3058HOPORTVZYY..XZVTQOGEDEEDGOQUVYL..YYZVTROPOHA503AK",
        "KA3058LUUUVcZZ....XZVTQOGEDEEDGPQUVYL....ZZcWUUTLA503AK",
        "KA3058LTTSVZdd....XYVTQOGDDEDDGPQTVYL....YaZWTTTLA5038K",
        "KA3058LTVZZY........fYROGDDEEDGPRZW........ZYZWTLA503BK",
        "KA3058LTWYYY........fcROGEEDDDGORZW........YYZWTLA503AK",
        "KA305ALcYY..........fcROGEEDDDGORYW..........ZcZLA5138K",
        "KA3058LYZb..........fcRPGEEDEEGORcW..........ccZLA513AK",
        "KA305ALZ...............fRPPPPPRbL..............YL8503BK",
        "K8305ALY...............cRPOPPORcL..............cL85138K",
        "KA76...................bROOOPORYM..................67AK",
        "KA8A...................bRPPOPPRcM..................8AAK",
        "........................IVTVVVI........................",
        ".........................cZZcbI........................",
    }
};

const EncodedSprite enemyWeaver = {
    { // palette
        0xf8f8f8, 0xefefee, 0xa1a1a1, 0x6b7073, 0x5e6569, 0x4b5255, 0x4a4d50, 0x3f4448, 0x3a3f43, 0x393d42, 0x383d42, 0x383d41, 0x373d41, 0x943135, 0x383c40, 0x373b40, 0x34383d, 0x32363a, 0x303438, 0x2e3236, 0x292e31, 0x5c1517, 0x212529, 0x212528, 0x212428, 0x202528, 0x202428, 0x1f2124, 0x1d2124, 0x1a1e21, 0x171a1d, 0x151718, 0x151618, 0x151615, 0x141618, 0x141617, 0x121416, 0x0f1012, 0x080909, 0x070708, 0x060708, 0xe80006, 0xb90003
    },
    { // rows
        ".............NQQGBBBBEEBEIQQQ.............",
        ".............MQQGBBBBBEBBIQQQ.............",
        ".............OOOGEBBBBBBEIQOR.............",
        "..........ZZRfgffffDBBDffffggaZZ..........",
        "..........WYRgggfffDBBDfffgggYWV..........",
        "..........YYRgggfffDBBDfffgggYWV..........",
        "..........WYRggggffDBBDffggggYYW..........",
        "..........WYRggggggDBBDggggggYWW..........",
        "..........WYRggggggDAADggggggYYW..........",
        "....HHHHHKYYULLLDDD69B6DDDLLLYWVHHHHHH....",
        "...GBBBBBHWWYYVVHAABBBABBJYYYYYYEEEBBB....",
        "...GBBBBEHYYYYWWH9ABBAAAEJWYYYYYBEBEBE....",
        "...JJJJJJKaabYYYE77655677HYYYabaIJJJJJF...",
        "GBBKYWWYYaddcYYY7664334668ZYZdedVYYYYZIBB7",
        "GBBKYYYYZaddcYWY7664334668YYZddcVYYYYYIBB7",
        "GBBKWY6333333aaa7664334668aaJ333333GYYIEB7",
        "GBBKYY2111000edd766433466Bee40001113YYIBB7",
        "GBBKWW2111000eed7664334669ee40001113WYIBE7",
        "GB9KVV2111000eee7664334558ee40001113VVIBE7",
        "SMNYdd2001898VYYSMN5334MMSWWT8880003deTNNK",
        "SPQYde2001A98VVYSPP5334PQTVVTAAB0003deTQNK",
        "SQQYdd2001AA8VWVSNN5334MNTVWTAAB0003ddTQNK",
        "SNPS..3333MMMbbb...6778...bbbMMN333...TNNK",
        "SNN....BBHVYVddd...FNNS...eecYWVABB...SPNJ",
        "SMM....EEHVVVddd...FMMS...edcWWVABB...TNNK",
        ".......BBHWWa................YWVABB.......",
        ".......BBHVVa................YVV9BC.......",
        ".......EBHVVa................WWV9BE.......",
        ".......IIKbbb................bbbHII.......",
        ".......NNSeee................eddOQQ.......",
        ".......MPSede................dddMQP.......",
        ".......MNSeeX................dedQQO.......",
        ".......MPQ......................SQP.......",
        ".......NPQ......................SPQ.......",
        ".......III......................JII.......",
    }
};

const EncodedSprite boss = {
    { // palette
        0x50d9d0, 0x2b8484, 0x34494a, 0x363d3e, 0x313738, 0x272f30, 0x252b2d, 0x23292b, 0x212729, 0x1e2527, 0xa9211e, 0xa7201e, 0x981e1d, 0x591d1e, 0x1f2123, 0x1b2123, 0x191f20, 0x171c1e, 0x151b1d, 0x871717, 0x7f1515, 0x7b1515, 0x6e1515, 0x481516, 0x351617, 0x2c1718, 0x211618, 0x161a1b, 0x141a1b, 0x161718, 0x13191a, 0x12191b, 0x13181a, 0x121819, 0x111819, 0x121719, 0x111718, 0x121617, 0x111617, 0x101517, 0x101415, 0x621111, 0x5a1010, 0x351112
    },
    { // rows
        "...................ccc.........................bccb...................",
        "..................Hcddd........................aaaYe..................",
        ".................eMAAJMd......................cMJAAMd.................",
        "................eMCACLgd......................cgLCACLd................",
        "................bBABLfNe......................eNfLBAAX................",
        "................ZBCKgOb........................cPfLCAV................",
        "..............ZNfJLfQ............................PfLJMNc..............",
        "..............UNKKMgd............................WgMKKNZ..............",
        ".....cdcd.....UNKKMgd............................WgMKKNZ.....USSI.....",
        "....PJMfNQe...UNKKMgd..........ecdeeedd..........WgMKKNW...UPNMLCT....",
        "...ePAJLgPe...UNLLfNd.........QPOOOOOPP..........ZNfLLNX...RPfLCARe...",
        "...ePACLgPd...Whgghb.........cNKAAAAAAKNW.........dOggOX...RPgLCAQe...",
        "...ePACLgPe...WhgghZ........RcNKAAAAAAKNZI........dOggOX...RPfLCAQe...",
        "...cPAJMgPd...WhgghT.......RddOMCCCCCCMOacG.......QOggOX...IPfLCAQe...",
        "...eQKMfgPe....eTgggc.....ccdcbTTTTTTTTTcXZW.....ZgggT.....IPffMJHe...",
        "....POOOOQ.....ecggghP...QNNNNDMMMMMMMMDNNNhQ..RPhgggW......POOON.....",
        ".....cddc......edggggPe.GTgMLLJAAAAAAAACLLMgQW.eOggfgZ.......SIHH.....",
        ".....Z98c........eOggPdacQgMLLJAAAAAAAACLKMgQZXcOggOb........S66G.....",
        ".....X87c.........QPgPdeeTgMLNNDDDDDDDDNNLMgQdddOgOQ.........S66G.....",
        ".....Z87c.........eefPeddTNfMQWXWWWWWZZZQMfgTccdOfab.........S66G.....",
        ".....a87c.........edddddddddc999999999999Zccddddddcb.........S66G.....",
        "....ecHGcb.......ehNNQddONTIF9ODDDDDDDDO9GRTNOdcPNNhe......WSIFFHW....",
        "...eTPQQQba......QNggOQPfMTG87DKKKKKKKKD88GTMgQTOggNQ......HHQQQQIE...",
        "...ePAJLgPc....ebgggdNMBJMTG7MKKKKKKKKKKM7GTMJBMNagggS.....HOfLJAEE...",
        "...ePAJLgPc...WhNgNONKCBJMTG8ODKKKKKKKKDN7GTMJACLNPNgNPR...HOfLCAEE...",
        "...ePCLMgQc...UONNOQLCBBCLPG98NMKKKKKKMN89EPLCAACKdOggPR...IPgMLCEE...",
        "...eXXZaaccZaZaccddQLCBBBCfhX988LKKKKL889WNfBBBBCKaaaZWUUSIIIIIIHI9...",
        "...eQDDNhQWE99EFFcdQLJCABBJMNGF8NNNNNN8FENMCBAACJKZaXF99988GQONDDG9...",
        "...eQAJLfPH555567adQMMLCBBBCLPG99999999GPLCAABCLMLaaW755444GOfLCAE9...",
        "...eQAJLfQG322247aabTTTQLAABBLNaXXXXZXXNMBBAAMQQQQYXX7422229OfLCAE9...",
        "...eQAJLgPH44447GGSZGGGGNDDJBCMNDDDDDDNMJAJDDNGGGFXIGF53333FOfLCAE9...",
        "...eQAJLfPI6666Gc8GU8998IacDBBJMBBBBBBLCAADXWG8887aG8XE6555GOfLCAE9...",
        "...eQKMfgPXIIRRUc8GU8211266EPBBBBBBBBBBBCPE5521127XG8WSHHGGHOffLJG9...",
        "...eTNNOOQceeeXHG8GU91111124EDJBBBBABAAJDG42111128WG8GHUdZWIQOONDHE...",
        "...eZZaaZcd...IF77GU811000129UDABBBAAAADS921000128UG77FU...ISIIHHIE...",
        ".....X87X.....IF77GXW210000016FQBCLLCCQE510000012SXF76FU.....I56G.....",
        ".....X86Z.....IF77GZa2100000125aJJLLJJW5210000012VXF76FU.....I56G.....",
        ".....W87Z.....RF77GXZ2100000012cLKLLLMU2100000012VXF77FU.....H57G.....",
        ".....XZZY......da8GXY2100000002a543346S1000000012UXF8UE......IIIH.....",
        "...............cX89Ga5211000012c753358W2100001125VF98S9...............",
        "...............ZV878UX911111112a853359V211111119SS788UE...............",
        ".........ISUUSSSUUF77GXUVUVVSSVd333334UUUUUUISVUF77FUUSIIHHIS.........",
        ".......ZWOOWHFEEESF77EGGPOOQXXG73333337GXXQOOPGF977FUEE99GRNNSS.......",
        "......EQQggT95555HF77777NffOXU7333333337SXOffN77768FI55558HMDEEQ......",
        ".....UMCCCJT522226FH776MLLfOU853333333359UOfLLM676SF722225HCBCCDH.....",
        ".....DJABCJT533335FSEODKKLfOU853345543358UOfLKKDO9UF633335HJBAAJD.....",
        "...eQABBBJMT955555GUWgKKKLfOU85337XX53358SOfLKKKNVUF655559HMCAAAAQb...",
        "...eQAABKMgTRHHHHHSXXDKKKMPTU743FIXXHE358UTPMKKKNWWSHHHGGHRgMJBAAQc...",
        "...NDACCMNNGFFGFFXWXXgKKKMTZU854SXXVUH458SWRMKKKNXXUUFFFFFENgMCBADN...",
        "..ULAAJLgPa......cXZXgKKKMTZU866SXXXWS668UWTMKKKNXXWa......HOfLCAADS..",
        "..WLAAJMQ........aG9ZNfKKMTZWIE7SXXXVI6EIXWTMKKfNX8Ga........QLCAADS..",
        "..OJBCKfZ........H98ZONMMfTXXXG9UXXWUS9GUUURMLMNOX69G........IMKCALO..",
        "GVKCBKMgc......dX768aZQgggTXZXXXXXXXUUUUUXUTggfQUX666S.......SgMLBCKVE",
        "FVKCBKOT......F974577GZTNgTH.ZF8SXXVUI7FU.FRgNRS9664369H......ROLBCKVE",
        "GVKJCMOa......H953457GXXONT..WF7FHUXHF6FU.GRNOUU9654348U......SOMCJKVE",
        "GVKKLfPW......H633347GWRRII..XF66FUU966FS.GIIIIS9633336S......IOfLLKVE",
        "GRfggV......UI333357aXZ......WF66FUU966FU......USV643333IR......VggfVE",
        "GRffgU......UH433457UZ........G969WU869F........WU654334HI......IggfVE",
        "GTfggX......RG333677W..........U69..57S..........W665334GH......SggfVE",
        "GTfOQ......R743456FI............Gb..ER............I9554346I......QOfVE",
        "GQfPW......S633469GU............I....d............YG854335I......SOfVE",
        "GTfPZ......I53346SUW..............................XSH54336I......IOfSE",
        "GWQQ.......S63346Ic................................cH54335I.......QPS9",
        ".ee........I53346Se................................eH54335I........cc.",
        "...........I53346Ie................................eH54335I...........",
        "...........I853468Hc..............................cF754358I...........",
        "...........XF63466FU..............................U9554369W...........",
        "............WI6666FU..............................U95556HI............",
        "............WI6666FU..............................U95556HI............",
        ".............R8766FU..............................U95578S.............",
        "..............H966FS..............................U9559S..............",
        "..............H9669U..............................S9559I..............",
        "..............RF968G..............................F758FU..............",
        "...............cU666I............................S556I................",
        ".................I96S............................S59H.................",
        ".................UF9S............................I8FI.................",
        "..................cIS............................IHd..................",
    }
};

const EncodedSprite bullet = {
    { // palette
        0xffffff, 0xfed3d3, 0xfec6c6, 0xffbfbf, 0xfcb4b4, 0xfbacac, 0xff9f9e, 0xfb9a9a, 0xfe9494, 0xff8e8e, 0xff8a8a, 0xff8383, 0xf98585, 0xfb7e7e, 0xf97878, 0xf16e6e, 0xf16464, 0xf15454, 0xeb3d3d, 0xf11918, 0xf10a0b, 0xf70505, 0xdf0809, 0x961616, 0x670404, 0xff0000, 0xfd0000, 0xf10100, 0xe60101, 0xd60000, 0xcd0101, 0xc40000, 0xb40000, 0xae0000, 0xa50000, 0x990000, 0x8f0000, 0x7c0001, 0x5b0102, 0x550203, 0x550000, 0x520304, 0x510000, 0x4c0304, 0x410204, 0x370204, 0x280104, 0x210104
    },
    { // rows
        "...............lllkiiiOOOOOOhiikllllll...",
        "...............lllkiiiOOOOOOhiikllllll...",
        "...............lllkiiiOOOOOOhiikllllll...",
        "...eeeaaaZYYXWWXZZYXXXUUUTTTTUUUVVXbbjlll",
        "...eeeaaaZYYXWWWVVUTTTRRRQQQQQQQQQTZZjlll",
        "...eeeaaaZYYXWWWVVUTTTRRRQQQQQQQQQTZZjlll",
        "gggNIIFFFGHHGGGGGGFEEEDDDEEEHIIJPPRMMhjjj",
        "gggIII999BEEDBBBAAA666555777GHHJPPLKKciii",
        "gggIII999BEEDBBBAAA666555777GHHJPPLKKciii",
        "cccCAA3334443444332222111222CEEJQQLKKOfff",
        "bbb8880000000000000000000000244IRRLKKbOOO",
        "bbb8880000000000000000000000244IRRLKKbOOO",
        "eeeHGG21132232231121111111117CCJRRLKKOddd",
        "gggNJJE99BEEDBBBAAA666555777GHHJPPLKKciii",
        "gggNJJE99BEEDBBBAAA666555777GHHJPPLKKciii",
        "gggbNNHEEGGGHFFGEEFAAA888CCCHIIKPPRMMdjjj",
        "...eeebaaZYYXWWWVVUQQQQQQQQQQQQQQQTZZjlll",
        "...eeebaaZYYXWWWVVUQQQQQQQQQQQQQQQTZZjlll",
        "...eeebaaZYYXWWXXXYSSSSSSSSSaaabbbOddjlll",
        "...............lllkiiiOOOOOOiiilllllll...",
        "...............lllkiiiOOOOOOiiilllllll...",
        "...............lllkiiiOOOOOOiiilllllll...",
    }
};

const EncodedSprite enemyBullet = {
    { // palette
        0xfecfce, 0xffcac9, 0xffc9c9, 0xfe8686, 0xee5f5f, 0xff5354, 0xff4040, 0xfe3132, 0xe84343, 0x852c2d, 0xfd201f, 0xfd1a1a, 0xf80d0c, 0xfb0806, 0xf30808, 0xf00202, 0xd11717, 0xcb0505, 0xa90707, 0xfd0000, 0xed0000, 0xdf0000, 0xd20000, 0xbe0000, 0xb10000, 0xaf0000, 0xa20000, 0x9e0000, 0x990000, 0x930000, 0x890000, 0x7e0000, 0x710000, 0x620000, 0x4e0000, 0x4d0000, 0x4c0000, 0x4b0000, 0x4a0000, 0x490000, 0x480000, 0x470000, 0x450000
    },
    { // rows
        ".........ddYYYZbb.......",
        ".........eeYZYZZa.......",
        "........IQQIIIIIII......",
        "..deSROOMFFEEEEEEHOTeg..",
        "..VV9G8444444447CMNQXX..",
        "cVKK332111222224BCJLRTcc",
        "dVKK332000000004ACJLRTcc",
        "dVKKDDAA7665566ABELHQTcc",
        "cVKKDDAA7665566ACELMQSbc",
        "..UUSSNHGGA77AACDHQUXX..",
        "..aaWWQOMLLJJJJJJNUWce..",
        "........ISROOPPOOQ......",
        ".........ddYZaaaa.......",
        ".........efZabcbb.......",
    }
};

const EncodedSprite powerupBlue = {
    { // palette
        0xcdf8ff, 0xcbf8ff, 0xcbf8fe, 0xcbf7ff, 0xcaf8ff, 0xbbebfe, 0xaee3fe, 0x9ddbfe, 0x9cd9fe, 0x7acefe, 0x2dc0ff, 0x2dbfff, 0x2dbffe, 0x2cc0ff, 0x2cbfff, 0x2bbfff, 0x2dbeff, 0x2dbefe, 0x2cbeff, 0x2cbefe, 0x2bbeff, 0x2bb9ee, 0x29b6f0, 0x29b1e2, 0x26a8d8, 0x1d91c0, 0x1577a0, 0x12607b, 0x053d5e, 0x003d5f, 0x003d5e, 0x083c5c, 0x003c5f, 0x003c5e, 0x003c5d, 0x02395a, 0x003b5e, 0x003b5d, 0x003b5c, 0x003a5d, 0x00395a, 0x003759, 0x003556
    },
    { // rows
        ".........gZeZZeZeeeeeZZeeZZf.........",
        ".........fTXXWTTXXTWTWXXWTWX.........",
        ".........fWXXXWWXXUXXXXXXXXX.........",
        ".........fdddddbbddddddddddd.........",
        ".....ffffNLLLLLLLLLLLLLLLLLLRfffZ....",
        ".....XXXbMIIIIIIGIIIIIGIIIIBRXXXS....",
        ".....XXXcMIIEIIEIEIEBEEIIIEBRXXWS....",
        ".....YbbcMEIIIIBGBGGGBEIEEEBRbYaS....",
        "....POOOOJIIIIB9666669BIIEEEOOOOP....",
        "XXWXPIIIIGEIGBB8001127BIIEEEIIIGNbXXX",
        "YXXXPIIIEEEBBEG8011107BEEEEEIEGINbXXU",
        "bXXXPIIIIEBBBBE8311117BEIEEIIIEENXWUU",
        "YXXaPIIBBIEGBEG8111107EEEEIIIIIENXXWT",
        "cXXbPGIIIGIIBBB8311137GGGGGGIEBBNaXUT",
        "bXXaPEEBG7666665331115666666AIIBNXXUT",
        "bXXXPIEBH40133313341111111309BEBNXUUT",
        "cXXXPIEBJ43133131144113112309IIENXUUU",
        "bXXXPEIBI33331131131113313109GEENXXUT",
        "bXXYPIIBI33113113111313313309IIGNXXXT",
        "cXXXPIBBG33331033311111321309IIINXXXW",
        "cXXXPBGGG8888885314115777777AIIINXWUT",
        "bXXXPGBGBBBBBBB8131317BEEBBBBIIENXWUU",
        "cWXXPEEBGEBBBBG8311117EBBBBIGGIENXTTX",
        "cXXbPIIIIEIEEIG8111137BEEGIGIIIINXXXX",
        "dXXbPIGIEIEBBGB8311117BEEEEEIIEINXXXX",
        "dXXaPIIIIEIIIEG8031307EEIIEIIIIKNXXXX",
        "ggggPOOOOJIEEIG9555559EEIIIBNOOOPgggg",
        ".....XXXdMIEIIIBCHHGGBIIIIIARXXbV....",
        ".....XXXbMIIIIIIEEIIIIIIIIIARXXXS....",
        ".....XXXbMIIIIIIIIIIIIIIIKKBRXXXV....",
        ".....ffZfMFFFFKKFFFFFFFFFFFDRZfeV....",
        ".........QQQQQQQQQQQQQQQQQQQ.........",
        ".........fWXXXWXUXXXXXXXXXWY.........",
        ".........fTUXXXXXUXXXXXXXWXY.........",
        ".........eWXXXXXXXXXXXXXXXUX.........",
    }
};

const EncodedSprite powerupRed = {
    { // palette
        0xfe7065, 0xfe6e63, 0xfe6d62, 0xff6c62, 0xff6b61, 0xff6b60, 0xfe6b61, 0xff665c, 0xfe5d53, 0xff322c, 0x8d3938, 0xff140e, 0xff0a04, 0xff0701, 0xff0700, 0xff0600, 0xfe0600, 0xf80601, 0xf70601, 0xe30402, 0xd60303, 0xd40303, 0xcb0304, 0xa70106, 0x900007, 0x8d0007, 0x8c0007, 0x8a0008, 0x8a0007, 0x890008, 0x890007, 0x810106, 0x880007, 0x870007, 0x870006, 0x860007, 0x840005, 0x830005, 0x810006, 0x800005, 0x7f0004
    },
    { // rows
        ".........YYYYYYYYXXYYYYXXb.........",
        ".........TTTTTRRRTTTTTTTTa.........",
        ".........TTTTTRRTTTTTTTTTa.........",
        ".........ZZZZXXXZZZZZZZXZd.........",
        "....cZZaLIHHHIHHIHHHHHHHHIYZZa.....",
        "....WRRRKFFFFFFFFFFFFFFFFGORTU.....",
        "....XTTTKFFFFFFFFFFFFFFFFFORTT.....",
        "....YTTTKEFFFFFFFEFFFFFFFGPTTT.....",
        "deVYJJJJIFFFFF877777CFFFFFJJJJMdVVe",
        "WWTPFFGFFFFFFF855552CFFFFFFFFFMWTUa",
        "WTTPFFFFFFFFFF855552CFFFFFFEFFMTTTa",
        "UTTPFFFFFFFFFF855442CFFFEFFFFFMTTTb",
        "RTTPFFFFFEEFFF855541CEFFFFFFFFMTTTb",
        "UTTPFFFFFFFFFF844542CFFFFFFFFFMTTUa",
        "UTTPFFFF822122255543222210BEEFMWTUa",
        "UTTQFFFF845565444455555551BFFFMWTUb",
        "TTTQFFFF855455544555455550BFEFMTTTb",
        "TTTQFFFF865565544444555551BFFFMTUTb",
        "WTTPFFFF855544544646555440BFFFMTTTa",
        "WTRPFFFF812111344444323210BFFEMUTTb",
        "UTRPFFFFB99999845551999999CFFEMTTTb",
        "TTRPFFFFFFFFFF855551CFEEFFFFFFMTTTb",
        "TTRPFFFFFFFFFF855551CEFEFFFFFEMTTTb",
        "TTRPFFFFFFFFFF855551CEEFFFFFFFMTTWb",
        "UTRPFFFFFFFFFF855561CEFFFFFFEFMTTTb",
        "UWUPFGFGGFFFFF855552CFEEEFGFEFMWWWb",
        "....NNNNJFFFFF999999DFFFFGNNNNA....",
        "....ZTTTKFFEFFFEFFFFFFFFFGORST.....",
        "....ZTTUKFFFFFEFFFFFFFFFFGORTT.....",
        "....ZWWYKGFFEFFFFFFEFFGFFGPUUU.....",
        "........MLLKLKLLKLLKLKLKKL.........",
        ".........RRRRRRRRRRRRRRRRZ.........",
        ".........TTTTRRTTTTTTRRTTa.........",
        ".........TTTTTTTTTTTTTTTTb.........",
        ".........ddcccVcddccccccce.........",
    }
};

const EncodedSprite powerupYellow = {
    { // palette
        0xffe487, 0xffe481, 0xffe381, 0xfee381, 0xffe380, 0xfee380, 0xffe37f, 0xfeda7c, 0xfac774, 0xfdbb60, 0xfa9715, 0xfd9000, 0xfc9001, 0xfc9000, 0xf99000, 0xfc8f00, 0xfc8e00, 0xf48f01, 0xdc7a00, 0xd77801, 0xd37600, 0xc96e03, 0xb65f00, 0x9d4a18, 0x933c14, 0x973800, 0x963700, 0x953700, 0x963600, 0x953600, 0x943500, 0x933500, 0x923201, 0x8f340c, 0x903000, 0x912f00, 0x8d2f00, 0x8f2e00, 0x8c2d01
    },
    { // rows
        ".........OQSSQQQQQQQQQQQTQN........",
        ".........OTTTTSTTSTSTTTTTSN........",
        ".........OTTTTSSSSSSSSSTSSN........",
        ".....aaaXKKKKKKKKKKKKKKKKKLccc.....",
        ".....TTTWEDDDDDDDDDDDBBDDDJTTTW....",
        ".....TTTVEDDDDDDDDDDDDDDDDJTTTW....",
        ".....TTTVEDDDDDDDDDDDDDDDDJUUTW....",
        "....MJJJJEDDDDD999999DDDDDHJJJK....",
        "ZTUTMFFFFDDDDDD724469DDDDDDDDFHVTTQ",
        "ZTTTMFFDDDDDDDD722449DDDDDDDDDHUTTQ",
        "ZTTTMFDDDDDDDDD744429DDDDDDDDDHVSTP",
        "WTTTMFDDFDDDDDD724449DDDDDDDDDHUTSP",
        "ZTTTMFDFFAAAAAA744469GGGGGGDDDHURSP",
        "WTTTMFDFG000011224440000008DDDHVTTP",
        "ZSTTMFDDG012222222442444148DDDHVSTP",
        "ZSTTMFDDG011242222441444248DDDHVSTP",
        "ZTTTMFDDG022242122423444248DDDHVTTP",
        "ZTTTMFDDG044444424424444428DDDHVTTP",
        "WTTTMFDDG888888645427888889DDDHUTTP",
        "ZTTTMDDDDFDDDDC744429DDDFFFFDDHVTTP",
        "ZTTTMDDDDDDDDDD746669FDDDDDDDDHVTTP",
        "ZTTTMDDDDDDDDDD726649FDDDDDDDDHVTTP",
        "ZTSTMFDDDDDDDDD746429FDDDDDDDDHVTTP",
        "ZTTTMDDDDDDDDDD724219FDDDDDDDDHURQP",
        "....MLLLLEDDDDD999999DDDDDILLLL....",
        ".....SSTVEDDDFDDDDDDDDDDDDJTTTW....",
        ".....QTTVEDDDDDDDDDDDDBBBDJTTTW....",
        ".....QSTVEDDDDDDDDDDDDBDDDJTTSW....",
        ".........JIIIIIIIIIIIIIIIIL........",
        ".........OSSTTTTSTSSTTSTSTN........",
        ".........OSTTSSSSTTTTTTTTTN........",
        ".........OSSSSTTTTTTTTTTTUN........",
        ".........XZYYZZbbYYYZZZYZb.........",
    }
};

PixelArt::Grid decodeSprite(const EncodedSprite& s, PixelArt::Palette& outPalette) {
    static const std::string CHARS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    outPalette.clear();
    outPalette.push_back(0); // 0 = transparente
    for (uint32_t c : s.palette) {
        outPalette.push_back(c);
    }

    PixelArt::Grid grid;
    for (const auto& row : s.rows) {
        std::vector<uint8_t> r;
        r.reserve(row.size());
        for (char ch : row) {
            if (ch == '.') {
                r.push_back(PixelArt::EMPTY);
            } else {
                size_t idx = CHARS.find(ch);
                if (idx != std::string::npos) {
                    r.push_back(static_cast<uint8_t>(idx + 1));
                } else {
                    r.push_back(PixelArt::EMPTY);
                }
            }
        }
        grid.push_back(std::move(r));
    }
    return grid;
}

} // namespace OriginalSprites
