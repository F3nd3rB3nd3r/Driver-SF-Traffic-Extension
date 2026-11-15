// Audited retail-image instructions, including the fourth scheduler vector.
struct StreamLayoutOperand {UPtr site;U32 original;UPtr instruction;U32 length,signature;};
static const StreamLayoutOperand kStreamLayoutOperands[]={
    {0x55E9D4u,0x16608u,0x55E9D0u,8u,0x269CE894u}, // subss       xmm0,dword ptr [eax+00016608h]
    {0x55E9DAu,0x16604u,0x55E9D8u,6u,0x4CBE6411u}, // mov         edx,dword ptr [eax+00016604h]
    {0x55E9E7u,0x1660Cu,0x55E9E3u,8u,0x5C3B6A48u}, // subss       xmm1,dword ptr [eax+0001660Ch]
    {0x563890u,0x16604u,0x56388Eu,6u,0x39B60449u}, // mov         ecx,dword ptr [eax+00016604h]
    {0x5638A3u,0x16608u,0x56389Fu,8u,0x269CE894u}, // subss       xmm0,dword ptr [eax+00016608h]
    {0x5638B1u,0x1660Cu,0x5638ADu,8u,0x924A28D0u}, // subss       xmm2,dword ptr [eax+0001660Ch]
    {0x58DDB9u,0x165CCu,0x58DDB7u,6u,0x22FA05E8u}, // mov         eax,dword ptr [eax+000165CCh]
    {0x590289u,0x165C0u,0x590288u,5u,0x5A334B1Bu}, // mov         esi,165C0h
    {0x5902ACu,0x165C8u,0x5902AAu,6u,0x3A28480Bu}, // cmp         edi,dword ptr [ecx+000165C8h]
    {0x592487u,0x165C8u,0x592485u,6u,0xB05D9994u}, // mov         esi,dword ptr [eax+000165C8h]
    {0x59249Eu,0x165C0u,0x59249Du,5u,0xADCE748Fu}, // mov         edx,165C0h
    {0x592537u,0x165C8u,0x592535u,6u,0xB05D9994u}, // mov         esi,dword ptr [eax+000165C8h]
    {0x59254Eu,0x165C0u,0x59254Du,5u,0xADCE748Fu}, // mov         edx,165C0h
    {0x59548Bu,0x165C8u,0x595489u,7u,0xEDB64713u}, // cmp         dword ptr [edi+000165C8h],0
    {0x5954A0u,0x165CCu,0x59549Eu,6u,0xD5407F55u}, // mov         eax,dword ptr [edi+000165CCh]
    {0x595891u,0x165C0u,0x59588Eu,7u,0xC37436ECu}, // mov         esi,dword ptr [eax+esi*4+000165C0h]
    {0x5962BCu,0x165CCu,0x5962BAu,6u,0x22FA05E8u}, // mov         eax,dword ptr [eax+000165CCh]
    {0x59E6C1u,0x16604u,0x59E6BFu,6u,0x4CBE6411u}, // mov         edx,dword ptr [eax+00016604h]
    {0x59E6CDu,0x16608u,0x59E6C9u,8u,0x269CE894u}, // subss       xmm0,dword ptr [eax+00016608h]
    {0x59E6D8u,0x16568u,0x59E6D6u,6u,0xC66FB79Cu}, // mov         ecx,dword ptr [eax+00016568h]
    {0x59E6E5u,0x1660Cu,0x59E6E1u,8u,0x5C3B6A48u}, // subss       xmm1,dword ptr [eax+0001660Ch]
    {0x59E879u,0x16568u,0x59E877u,6u,0xC66FB79Cu}, // mov         ecx,dword ptr [eax+00016568h]
    {0x59E87Fu,0x16604u,0x59E87Du,6u,0x4CBE6411u}, // mov         edx,dword ptr [eax+00016604h]
    {0x59E898u,0x1660Cu,0x59E894u,8u,0x924A28D0u}, // subss       xmm2,dword ptr [eax+0001660Ch]
    {0x59E8A4u,0x16608u,0x59E8A0u,8u,0x269CE894u}, // subss       xmm0,dword ptr [eax+00016608h]
    {0x59E9F9u,0x16568u,0x59E9F7u,6u,0xC66FB79Cu}, // mov         ecx,dword ptr [eax+00016568h]
    {0x59E9FFu,0x16604u,0x59E9FDu,6u,0x4CBE6411u}, // mov         edx,dword ptr [eax+00016604h]
    {0x59EA18u,0x1660Cu,0x59EA14u,8u,0x924A28D0u}, // subss       xmm2,dword ptr [eax+0001660Ch]
    {0x59EA24u,0x16608u,0x59EA20u,8u,0x269CE894u}, // subss       xmm0,dword ptr [eax+00016608h]
    {0x59EB74u,0x16608u,0x59EB70u,8u,0x269CE894u}, // subss       xmm0,dword ptr [eax+00016608h]
    {0x59EB81u,0x1660Cu,0x59EB7Du,8u,0x924A28D0u}, // subss       xmm2,dword ptr [eax+0001660Ch]
    {0x59EB87u,0x16604u,0x59EB85u,6u,0xE249E7E1u}, // mov         eax,dword ptr [eax+00016604h]
    {0x59EBFCu,0x16604u,0x59EBFAu,6u,0x4CBE6411u}, // mov         edx,dword ptr [eax+00016604h]
    {0x59EC10u,0x16608u,0x59EC0Cu,8u,0x269CE894u}, // subss       xmm0,dword ptr [eax+00016608h]
    {0x59EC2Eu,0x1660Cu,0x59EC2Au,8u,0x5C3B6A48u}, // subss       xmm1,dword ptr [eax+0001660Ch]
    {0x59EC3Du,0x16608u,0x59EC39u,8u,0xBC286C64u}, // subss       xmm2,dword ptr [eax+00016608h]
    {0x59EC49u,0x1660Cu,0x59EC45u,8u,0xF1C6EE18u}, // subss       xmm3,dword ptr [eax+0001660Ch]
    {0x59ECB7u,0x16604u,0x59ECB5u,6u,0x5E9E788Fu}, // mov         eax,dword ptr [edx+00016604h]
    {0x59ECC4u,0x16608u,0x59ECC0u,8u,0xFBDFE71Eu}, // subss       xmm0,dword ptr [edx+00016608h]
    {0x59ECD4u,0x1660Cu,0x59ECD0u,8u,0x4E40725Au}, // subss       xmm2,dword ptr [edx+0001660Ch]
    {0x59ED0Fu,0x16568u,0x59ED0Du,6u,0x8BE3D2EEu}, // add         ecx,dword ptr [edx+00016568h]
    {0x59EE35u,0x16608u,0x59EE31u,8u,0x269CE894u}, // subss       xmm0,dword ptr [eax+00016608h]
    {0x59EE3Bu,0x16568u,0x59EE39u,6u,0xBFCD3694u}, // mov         edx,dword ptr [eax+00016568h]
    {0x59EE43u,0x1660Cu,0x59EE3Fu,8u,0x924A28D0u}, // subss       xmm2,dword ptr [eax+0001660Ch]
    {0x59EE49u,0x16604u,0x59EE47u,6u,0xE249E7E1u}, // mov         eax,dword ptr [eax+00016604h]
    {0x59F110u,0x16604u,0x59F10Eu,6u,0xE249E7E1u}, // mov         eax,dword ptr [eax+00016604h]
    {0x5A2972u,0x165D4u,0x5A2970u,6u,0x7A53727u}, // mov         eax,dword ptr [ecx+000165D4h]
    {0x5A2980u,0x16614u,0x5A297Cu,8u,0x38197343u}, // movss       xmm0,dword ptr [ecx+00016614h]
    {0x5A2990u,0x16614u,0x5A298Cu,8u,0xAAAA1142u}, // movss       dword ptr [ecx+00016614h],xmm0
    {0x5A29A2u,0x165D4u,0x5A29A0u,6u,0x48F1CB0Fu}, // mov         ecx,dword ptr [ecx+000165D4h]
    {0x5A29CCu,0x16568u,0x5A29CAu,6u,0x1C7938F9u}, // mov         ecx,dword ptr [edi+00016568h]
    {0x5A29D6u,0x16604u,0x5A29D4u,6u,0x5426088Cu}, // mov         edx,dword ptr [edi+00016604h]
    {0x5A2A2Bu,0x165CCu,0x5A2A29u,6u,0x5EC4A87Fu}, // mov         eax,dword ptr [ecx+000165CCh]
    {0x5A2A7Eu,0x400u,0x5A2A7Cu,6u,0x221F764Du}, // mov         eax,dword ptr [ecx+00000400h]
    {0x5A2B11u,0x804u,0x5A2B0Fu,6u,0xDA74873Du}, // mov         edx,dword ptr [ecx+00000804h]
    {0x5A2B17u,0x404u,0x5A2B15u,6u,0x5E19097Bu}, // lea         edi,[ecx+00000404h]
    {0x5A2BB2u,0xC08u,0x5A2BB0u,6u,0xAC4BD47Du}, // mov         eax,dword ptr [ecx+00000C08h]
    {0x5A2BB8u,0x808u,0x5A2BB6u,6u,0x919A6D43u}, // lea         edi,[ecx+00000808h]
    {0x5A2C52u,0x400u,0x5A2C50u,6u,0x221F764Du}, // mov         eax,dword ptr [ecx+00000400h]
    {0x5A2CC7u,0x804u,0x5A2CC5u,6u,0x7BECA20Bu}, // mov         eax,dword ptr [edi+00000804h]
    {0x5A2CCDu,0x404u,0x5A2CCBu,6u,0xB11297F5u}, // lea         esi,[edi+00000404h]
    {0x5A2CE2u,0x1661Cu,0x5A2CE0u,7u,0x4B37EABDu}, // cmp         byte ptr [edi+0001661Ch],0
    {0x5A2CF7u,0x16560u,0x5A2CF5u,6u,0x49295171u}, // cmp         ecx,dword ptr [edi+00016560h]
    {0x5A2D17u,0x1655Cu,0x5A2D15u,6u,0xD23D8039u}, // mov         eax,dword ptr [ebx+0001655Ch]
    {0x5A2D62u,0x400u,0x5A2D60u,6u,0x5F96A8AFu}, // mov         ecx,dword ptr [ebx+00000400h]
    {0x5A2DA2u,0x165D4u,0x5A2DA0u,6u,0x83D31D81u}, // mov         eax,dword ptr [ebx+000165D4h]
    {0x5A2DB6u,0x16560u,0x5A2DB4u,6u,0x6454244Du}, // mov         ecx,dword ptr [ebx+00016560h]
    {0x5A2DD8u,0x165E4u,0x5A2DD6u,6u,0x3748B439u}, // mov         ecx,dword ptr [ebx+000165E4h]
    {0x5A2DF3u,0x165E4u,0x5A2DF1u,6u,0x3748B439u}, // mov         ecx,dword ptr [ebx+000165E4h]
    {0x5A2E0Au,0x16610u,0x5A2E08u,10u,0x51F2F1C2u}, // mov         dword ptr [ebx+00016610h],5A2830h
    {0x5A2E4Cu,0x165CCu,0x5A2E4Au,6u,0xCEB6A76Eu}, // mov         ecx,dword ptr [esi+000165CCh]
    {0x5A2F05u,0x16568u,0x5A2F03u,6u,0x4DED903Au}, // mov         ecx,dword ptr [esi+00016568h]
    {0x5A2F2Bu,0x165F8u,0x5A2F29u,6u,0x212D3672u}, // mov         eax,dword ptr [esi+000165F8h]
    {0x5A2F3Au,0x16570u,0x5A2F38u,10u,0xE71CEC88u}, // mov         dword ptr [esi+00016570h],0EA5C30h
    {0x5A2F6Au,0x165F8u,0x5A2F68u,6u,0x52AA3153u}, // mov         esi,dword ptr [ecx+000165F8h]
    {0x5A2F70u,0x16600u,0x5A2F6Eu,6u,0x65493526u}, // mov         edi,dword ptr [ecx+00016600h]
    {0x5A2F78u,0x165E8u,0x5A2F76u,6u,0x2A6D85BDu}, // lea         esi,[ecx+000165E8h]
    {0x5A305Eu,0x400u,0x5A305Cu,6u,0xB8386B7Bu}, // mov         eax,dword ptr [edi+00000400h]
    {0x5A3083u,0x804u,0x5A3081u,6u,0x351D98A3u}, // mov         ecx,dword ptr [edi+00000804h]
    {0x5A3089u,0x404u,0x5A3087u,6u,0xB11297F5u}, // lea         esi,[edi+00000404h]
    {0x5A30EEu,0x400u,0x5A30ECu,6u,0x3C56E5A0u}, // mov         ecx,dword ptr [esi+00000400h]
    {0x5A311Au,0xC08u,0x5A3118u,6u,0xACD945DBu}, // mov         edx,dword ptr [edi+00000C08h]
    {0x5A3120u,0x808u,0x5A311Eu,6u,0xCF119FA5u}, // lea         ecx,[edi+00000808h]
    {0x5A314Eu,0x804u,0x5A314Cu,6u,0x351D98A3u}, // mov         ecx,dword ptr [edi+00000804h]
    {0x5A3154u,0x404u,0x5A3152u,6u,0xDC299F95u}, // lea         edx,[edi+00000404h]
    {0x5A316Eu,0x400u,0x5A316Cu,6u,0x8D871394u}, // mov         esi,dword ptr [edx+00000400h]
    {0x5A318Eu,0x400u,0x5A318Cu,6u,0x124A2A20u}, // dec         dword ptr [edx+00000400h]
    {0x5A31ADu,0x165CCu,0x5A31ABu,6u,0x5A83D53Du}, // mov         dword ptr [ecx+000165CCh],edx
    {0x5A31B3u,0x165C8u,0x5A31B1u,6u,0x424FBED3u}, // mov         eax,dword ptr [ecx+000165C8h]
    {0x5A31BAu,0x165C0u,0x5A31B7u,7u,0xFC2F1591u}, // lea         eax,[ecx+eax*4+000165C0h]
    {0x5A31C6u,0x165C8u,0x5A31C4u,6u,0xF687FD77u}, // inc         dword ptr [ecx+000165C8h]
    {0x5A31DBu,0x165C8u,0x5A31D9u,6u,0x6FDEA4BDu}, // mov         eax,dword ptr [ebx+000165C8h]
    {0x5A31E2u,0x165C0u,0x5A31E0u,6u,0xB7415913u}, // lea         edi,[ebx+000165C0h]
    {0x5A3231u,0x165CCu,0x5A322Fu,6u,0x2C8A987Bu}, // cmp         dword ptr [ebx+000165CCh],ebp
    {0x5A3239u,0x165C8u,0x5A3237u,7u,0x5BE000C7u}, // cmp         dword ptr [ebx+000165C8h],0
    {0x5A3245u,0x165CCu,0x5A3243u,10u,0xD46DE9DDu}, // mov         dword ptr [ebx+000165CCh],0
    {0x5A3255u,0x165CCu,0x5A3253u,6u,0x8021D9A3u}, // mov         dword ptr [ebx+000165CCh],eax
    {0x5A326Fu,0x804u,0x5A326Du,6u,0x6127C15Au}, // mov         ecx,dword ptr [eax+00000804h]
    {0x5A3279u,0x404u,0x5A3278u,5u,0xF5E29858u}, // add         eax,404h
    {0x5A32AEu,0x16560u,0x5A32ACu,6u,0x15C250Bu}, // cmp         esi,dword ptr [ecx+00016560h]
    {0x5A3322u,0x165D0u,0x5A3320u,7u,0xE1CD59A1u}, // cmp         dword ptr [ecx+000165D0h],0
    {0x5A3329u,0x1661Eu,0x5A3327u,7u,0x4224D45Cu}, // mov         byte ptr [ecx+0001661Eh],1
    {0x5A3346u,0x1818u,0x5A3344u,6u,0x795DA78Cu}, // imul        eax,eax,1818h
    {0x5A3351u,0xC0Cu,0x5A334Eu,7u,0x147FDEBCu}, // lea         esi,[eax+ecx+00000C0Ch]
    {0x5A3398u,0x400u,0x5A3396u,6u,0xF52C695Au}, // mov         dword ptr [esi+00000400h],ebx
    {0x5A339Eu,0x804u,0x5A339Cu,6u,0xC1AB0592u}, // mov         dword ptr [esi+00000804h],ebx
    {0x5A33A4u,0xC08u,0x5A33A2u,6u,0x6B000B2Au}, // mov         dword ptr [esi+00000C08h],ebx
    {0x5A33AAu,0xC0Cu,0x5A33A8u,6u,0xF72C6E5Au}, // lea         eax,[esi+00000C0Ch]
    {0x5A33B8u,0x180Cu,0x5A33B6u,6u,0x32C470E4u}, // lea         ecx,[eax+0000180Ch]
    {0x5A33C4u,0x1810u,0x5A33C2u,6u,0x398ED18Cu}, // mov         dword ptr [eax+00001810h],ecx
    {0x5A33C9u,0x1818u,0x5A33C8u,5u,0x2C27ED30u}, // add         eax,1818h
    {0x5A33D6u,0x1655Cu,0x5A33D4u,6u,0x6FA5D688u}, // mov         dword ptr [esi+0001655Ch],ebx
    {0x5A33DCu,0x16570u,0x5A33DAu,6u,0x42721208u}, // lea         ecx,[esi+00016570h]
    {0x5A33E2u,0x16560u,0x5A33E0u,6u,0x720583FCu}, // mov         dword ptr [esi+00016560h],ebx
    {0x5A33E8u,0x16564u,0x5A33E6u,6u,0x672565A0u}, // mov         dword ptr [esi+00016564h],ebx
    {0x5A33EEu,0x16568u,0x5A33ECu,6u,0xAA505E34u}, // mov         dword ptr [esi+00016568h],ebx
    {0x5A33F9u,0x165C8u,0x5A33F7u,6u,0x13B31D4u}, // mov         dword ptr [esi+000165C8h],ebx
    {0x5A33FFu,0x165CCu,0x5A33FDu,6u,0x3CAE40B8u}, // mov         dword ptr [esi+000165CCh],ebx
    {0x5A3405u,0x165D0u,0x5A3403u,6u,0xF8BAC0ECu}, // mov         dword ptr [esi+000165D0h],ebx
    {0x5A340Bu,0x165E8u,0x5A3409u,6u,0x649D63C0u}, // lea         ecx,[esi+000165E8h]
    {0x5A3411u,0x165D4u,0x5A340Fu,6u,0x4D7A84D0u}, // mov         dword ptr [esi+000165D4h],ebx
    {0x5A3417u,0x165D8u,0x5A3415u,6u,0x7758C864u}, // mov         dword ptr [esi+000165D8h],ebx
    {0x5A341Du,0x165DCu,0x5A341Bu,6u,0x85C55F08u}, // mov         dword ptr [esi+000165DCh],ebx
    {0x5A3423u,0x165E0u,0x5A3421u,6u,0x88250C7Cu}, // mov         dword ptr [esi+000165E0h],ebx
    {0x5A3429u,0x165E4u,0x5A3427u,6u,0x7D44EE20u}, // mov         dword ptr [esi+000165E4h],ebx
    {0x5A3437u,0x165F8u,0x5A3435u,6u,0x368D7D44u}, // mov         dword ptr [esi+000165F8h],ebx
    {0x5A343Du,0x165FCu,0x5A343Bu,6u,0x44FA13E8u}, // mov         dword ptr [esi+000165FCh],ebx
    {0x5A3443u,0x16600u,0x5A3441u,6u,0xC08B13CDu}, // mov         dword ptr [esi+00016600h],ebx
    {0x5A3449u,0x16604u,0x5A3447u,6u,0xCEF7AA71u}, // mov         dword ptr [esi+00016604h],ebx
    {0x5A344Fu,0x1661Cu,0x5A344Du,6u,0xA8C2BCE8u}, // mov         byte ptr [esi+0001661Ch],bl
    {0x5A3455u,0x1661Du,0x5A3453u,6u,0xA0363FD5u}, // mov         byte ptr [esi+0001661Dh],bl
    {0x5A345Bu,0x1661Eu,0x5A3459u,6u,0xC67C445Au}, // mov         byte ptr [esi+0001661Eh],bl
    {0x5A3463u,0x16614u,0x5A345Fu,8u,0x62E9AD73u}, // movss       dword ptr [esi+00016614h],xmm0
    {0x5A3469u,0x16618u,0x5A3467u,10u,0x668E014Fu}, // mov         dword ptr [esi+00016618h],0FFFFFFFFh
    {0x5A3473u,0x16610u,0x5A3471u,10u,0x6C561879u}, // mov         dword ptr [esi+00016610h],5A2860h
    {0x5A347Du,0x165B8u,0x5A347Bu,7u,0xCE181D42u}, // mov         byte ptr [esi+000165B8h],1
    {0x5A34A4u,0x165D4u,0x5A34A2u,10u,0xCD4D9AEu}, // mov         dword ptr [esi+000165D4h],4
    {0x5A34DDu,0x16604u,0x5A34DBu,6u,0x3B9E87D5u}, // lea         edi,[esi+00016604h]
    {0x5A34ECu,0x165D0u,0x5A34EAu,6u,0xF8BAC0ECu}, // mov         dword ptr [esi+000165D0h],ebx
    {0x5A34F9u,0x165ACu,0x5A34F7u,10u,0xA29C0742u}, // mov         dword ptr [esi+000165ACh],0
    {0x5A355Cu,0x16568u,0x5A355Au,6u,0xDD31F03Cu}, // mov         dword ptr [esi+00016568h],eax
    {0x5A3578u,0x16568u,0x5A3576u,6u,0x20ABEDAAu}, // mov         ebx,dword ptr [esi+00016568h]
    {0x5A35A7u,0x165D0u,0x5A35A5u,6u,0x18ACC58Au}, // mov         eax,dword ptr [esi+000165D0h]
    {0x5A35CDu,0x165D8u,0x5A35CBu,6u,0x1D7BF294u}, // mov         dword ptr [esi+000165D8h],ecx
    {0x5A35D3u,0x165DCu,0x5A35D1u,6u,0x37A7A760u}, // mov         dword ptr [esi+000165DCh],edx
    {0x5A3609u,0x165D8u,0x5A3607u,6u,0xBBD55762u}, // mov         edx,dword ptr [esi+000165D8h]
    {0x5A3639u,0x165D8u,0x5A3637u,6u,0xE061EB52u}, // mov         eax,dword ptr [esi+000165D8h]
    {0x5A3644u,0x165E8u,0x5A3642u,6u,0x649D63C0u}, // lea         ecx,[esi+000165E8h]
    {0x5A364Fu,0x165F8u,0x5A364Du,7u,0x5BCE71B6u}, // cmp         dword ptr [esi+000165F8h],0
    {0x5A365Cu,0x165F8u,0x5A365Au,6u,0x9A2AD980u}, // lea         ebx,[esi+000165F8h]
    {0x5A3693u,0x16600u,0x5A3691u,6u,0xF2376E17u}, // mov         eax,dword ptr [esi+00016600h]
    {0x5A369Eu,0x165F8u,0x5A369Cu,6u,0xFCAAE7Au}, // add         eax,dword ptr [esi+000165F8h]
    {0x5A36B8u,0x165DCu,0x5A36B6u,6u,0x238CE3E6u}, // mov         eax,dword ptr [esi+000165DCh]
    {0x5A36D6u,0x165D0u,0x5A36D4u,6u,0x9C57F2F2u}, // mov         ecx,dword ptr [esi+000165D0h]
    {0x5A373Bu,0x16568u,0x5A3739u,6u,0xEE35A0A2u}, // mov         eax,dword ptr [esi+00016568h]
    {0x5A375Du,0x16568u,0x5A375Bu,6u,0x20ABEDAAu}, // mov         ebx,dword ptr [esi+00016568h]
    {0x5A37E7u,0x16608u,0x5A37E5u,6u,0x800C73BDu}, // mov         dword ptr [esi+00016608h],edx
    {0x5A37F1u,0x1660Cu,0x5A37EFu,6u,0x24048E31u}, // mov         dword ptr [esi+0001660Ch],eax
    {0x5A37FCu,0x16596u,0x5A37F9u,7u,0xB41681E2u}, // mov         word ptr [esi+00016596h],cx
    {0x5A380Du,0x16594u,0x5A380Au,7u,0x78AB129Cu}, // mov         word ptr [esi+00016594h],cx
    {0x5A381Cu,0x165F8u,0x5A381Au,7u,0x5BCE71B6u}, // cmp         dword ptr [esi+000165F8h],0
    {0x5A382Fu,0x165F8u,0x5A382Du,6u,0x1029860Cu}, // mov         dword ptr [esi+000165F8h],eax
    {0x5A3839u,0x165FCu,0x5A3837u,10u,0x85DDAEE3u}, // mov         dword ptr [esi+000165FCh],1
    {0x5A384Bu,0x165ACu,0x5A3849u,10u,0xA29C0742u}, // mov         dword ptr [esi+000165ACh],0
    {0x5A3856u,0x16596u,0x5A3853u,7u,0x416E3FDAu}, // mov         word ptr [esi+00016596h],dx
    {0x5A385Du,0x16594u,0x5A385Au,7u,0x630CD2E4u}, // mov         word ptr [esi+00016594h],ax
    {0x5A38DCu,0x16614u,0x5A38D8u,8u,0x5F497C28u}, // movss       dword ptr [ebx+00016614h],xmm0
    {0x5A38E2u,0x16560u,0x5A38E0u,6u,0x8F0FB90Fu}, // mov         dword ptr [ebx+00016560h],edi
    {0x5A38F8u,0x400u,0x5A38F6u,6u,0x5F96A8AFu}, // mov         ecx,dword ptr [ebx+00000400h]
    {0x5A395Cu,0xC08u,0x5A395Au,6u,0xD82E3B8Fu}, // mov         ecx,dword ptr [ebx+00000C08h]
    {0x5A3964u,0x808u,0x5A3962u,6u,0x65B80631u}, // lea         esi,[ebx+00000808h]
    {0x5A39D2u,0x400u,0x5A39D0u,6u,0xFF4562F7u}, // mov         edx,dword ptr [ebx+00000400h]
    {0x5A3A0Eu,0x400u,0x5A3A0Cu,6u,0x58F171E3u}, // dec         dword ptr [ebx+00000400h]
    {0x5A3A34u,0x804u,0x5A3A32u,6u,0x9BE2721Fu}, // mov         ecx,dword ptr [ebx+00000804h]
    {0x5A3A3Au,0x404u,0x5A3A38u,6u,0x55DC1531u}, // lea         ebp,[ebx+00000404h]
    {0x5A3AF2u,0x400u,0x5A3AF0u,6u,0xAFFB7731u}, // mov         edx,dword ptr [ebp+00000400h]
    {0x5A3B0Fu,0x400u,0x5A3B0Du,6u,0x4586FB01u}, // mov         eax,dword ptr [ebp+00000400h]
    {0x5A3B1Du,0x400u,0x5A3B1Bu,6u,0xAFFB7731u}, // mov         edx,dword ptr [ebp+00000400h]
    {0x5A3B42u,0x400u,0x5A3B40u,6u,0xC2D87CB5u}, // dec         dword ptr [ebp+00000400h]
    {0x5A3B52u,0x165D4u,0x5A3B50u,10u,0xF98973A6u}, // mov         dword ptr [ebx+000165D4h],3
    {0x5A3B75u,0x165D0u,0x5A3B73u,7u,0xE55DA37Eu}, // cmp         dword ptr [esi+000165D0h],0
    {0x5A3B88u,0x165D0u,0x5A3B86u,6u,0x9C57F2F2u}, // mov         ecx,dword ptr [esi+000165D0h]
    {0x5A3B8Eu,0x16604u,0x5A3B8Cu,6u,0x4CB776FDu}, // lea         eax,[esi+00016604h]
    {0x5A3BE3u,0xC08u,0x5A3BE1u,6u,0xC3BF52C0u}, // mov         ecx,dword ptr [esi+00000C08h]
    {0x5A3BE9u,0x808u,0x5A3BE7u,6u,0x3E2077AEu}, // add         esi,808h
    {0x5A3C1Bu,0x404u,0x5A3C19u,6u,0x8C010C6Eu}, // add         esi,404h
    {0x5A3C26u,0x400u,0x5A3C24u,6u,0x719123B8u}, // mov         eax,dword ptr [esi+00000400h]
    {0x5A3C88u,0xC08u,0x5A3C86u,6u,0xF8D1CF33u}, // mov         ecx,dword ptr [edi+00000C08h]
    {0x5A3C91u,0x808u,0x5A3C8Fu,6u,0x131CB7FDu}, // lea         eax,[edi+00000808h]
    {0x5A3C9Eu,0x400u,0x5A3C9Cu,6u,0x45ECAABAu}, // mov         edx,dword ptr [eax+00000400h]
    {0x5A3D29u,0x1818u,0x5A3D27u,6u,0xF27B3C57u}, // imul        ecx,ecx,1818h
    {0x5A3D30u,0xC0Cu,0x5A3D2Du,7u,0x2DE7467u}, // lea         edx,[ecx+edx+00000C0Ch]
    {0x5A3D36u,0x1810u,0x5A3D34u,6u,0x28E2EA78u}, // mov         ecx,dword ptr [edx+00001810h]
    {0x5A3D3Cu,0x180Cu,0x5A3D3Au,6u,0xDB9BD11Au}, // lea         esi,[edx+0000180Ch]
    {0x5A3DC2u,0xC08u,0x5A3DC0u,6u,0x4264C9ABu}, // mov         eax,dword ptr [edi+00000C08h]
    {0x5A3DC8u,0x808u,0x5A3DC6u,6u,0x99207BD1u}, // add         edi,808h
    {0x5A3EACu,0x1818u,0x5A3EAAu,6u,0x8C61701Bu}, // imul        ebp,ebp,1818h
    {0x5A3EB7u,0xC0Cu,0x5A3EB4u,7u,0x1E7B538Fu}, // lea         esi,[ecx+ebp+00000C0Ch]
    {0x5A3EBDu,0x1810u,0x5A3EBBu,6u,0x43800984u}, // mov         ecx,dword ptr [esi+00001810h]
    {0x5A3EC3u,0x180Cu,0x5A3EC1u,6u,0xA2B1F04Eu}, // lea         edx,[esi+0000180Ch]
    {0x5A3F6Cu,0x1818u,0x5A3F6Au,6u,0x795DA78Cu}, // imul        eax,eax,1818h
    {0x5A3F81u,0xC0Cu,0x5A3F7Eu,7u,0x922164ACu}, // lea         edi,[eax+esi+00000C0Ch]
    {0x5A3FB3u,0xC08u,0x5A3FB1u,6u,0x549D32C8u}, // mov         edx,dword ptr [esi+00000C08h]
    {0x5A3FB9u,0x808u,0x5A3FB7u,6u,0x2F8AA702u}, // lea         ebx,[esi+00000808h]
    {0x5A401Du,0x400u,0x5A401Bu,6u,0x719123B8u}, // mov         eax,dword ptr [esi+00000400h]
    {0x5A4092u,0x400u,0x5A4090u,6u,0xFF4562F7u}, // mov         edx,dword ptr [ebx+00000400h]
    {0x5A40AEu,0x400u,0x5A40ACu,6u,0x94D0E6C7u}, // mov         eax,dword ptr [ebx+00000400h]
    {0x5A40BBu,0x400u,0x5A40B9u,6u,0xFF4562F7u}, // mov         edx,dword ptr [ebx+00000400h]
    {0x5A40DEu,0x400u,0x5A40DCu,6u,0x58F171E3u}, // dec         dword ptr [ebx+00000400h]
    {0x5A4108u,0x165D4u,0x5A4106u,6u,0x976DC2FDu}, // mov         eax,dword ptr [edi+000165D4h]
    {0x5A4125u,0x16560u,0x5A4123u,6u,0xE42E5EC1u}, // mov         ecx,dword ptr [edi+00016560h]
    {0x5A4131u,0x16560u,0x5A412Fu,6u,0x50AE66FFu}, // lea         esi,[edi+00016560h]
    {0x5A413Eu,0x165D4u,0x5A413Cu,10u,0x5DFACEBDu}, // mov         dword ptr [edi+000165D4h],4
    {0x5A4170u,0x165D4u,0x5A416Eu,10u,0x5DFACEBDu}, // mov         dword ptr [edi+000165D4h],4
    {0x5A41D4u,0x804u,0x5A41D2u,6u,0x7BECA20Bu}, // mov         eax,dword ptr [edi+00000804h]
    {0x5A41DAu,0x404u,0x5A41D8u,6u,0xA6EF617Du}, // lea         ebx,[edi+00000404h]
    {0x5A41EBu,0x400u,0x5A41E9u,6u,0x5F96A8AFu}, // mov         ecx,dword ptr [ebx+00000400h]
    {0x5A4204u,0xC08u,0x5A4202u,6u,0x4264C9ABu}, // mov         eax,dword ptr [edi+00000C08h]
    {0x5A420Au,0x808u,0x5A4208u,6u,0x649D2375u}, // lea         ebx,[edi+00000808h]
    {0x5A4231u,0x400u,0x5A422Fu,6u,0x22ACE7ABu}, // mov         edx,dword ptr [edi+00000400h]
    {0x5A4258u,0x804u,0x5A4256u,6u,0x351D98A3u}, // mov         ecx,dword ptr [edi+00000804h]
    {0x5A425Eu,0x404u,0x5A425Cu,6u,0xA6EF617Du}, // lea         ebx,[edi+00000404h]
    {0x5A42A8u,0x16614u,0x5A42A4u,8u,0x1CC0B544u}, // movss       dword ptr [edi+00016614h],xmm0
    {0x5A42AEu,0x165D4u,0x5A42ACu,10u,0x5DFACEBDu}, // mov         dword ptr [edi+000165D4h],4
    {0x5A42BDu,0x165D0u,0x5A42BBu,6u,0x7F6FD5D1u}, // mov         ecx,dword ptr [edi+000165D0h]
    {0x5A42D0u,0x165E4u,0x5A42CEu,6u,0xF7ED19D5u}, // mov         ecx,dword ptr [edi+000165E4h]
    {0x5A42E6u,0x16614u,0x5A42E2u,8u,0x1CC0B544u}, // movss       dword ptr [edi+00016614h],xmm0
    {0x5A42ECu,0x165D4u,0x5A42EAu,10u,0x5DFACEBDu}, // mov         dword ptr [edi+000165D4h],4
    {0x5A42F6u,0x165E4u,0x5A42F4u,6u,0x169CE55Fu}, // mov         dword ptr [edi+000165E4h],eax
    {0x5A42FCu,0x165E0u,0x5A42FAu,6u,0x1374B0B3u}, // mov         dword ptr [edi+000165E0h],eax
    {0x5A4307u,0x165E0u,0x5A4305u,6u,0xCE0ED641u}, // mov         ecx,dword ptr [edi+000165E0h]
    {0x5A431Au,0x165E4u,0x5A4318u,6u,0xF7ED19D5u}, // mov         ecx,dword ptr [edi+000165E4h]
    {0x5A4321u,0x165E0u,0x5A431Fu,6u,0xCE0ED641u}, // mov         ecx,dword ptr [edi+000165E0h]
    {0x5A432Cu,0x165D4u,0x5A432Au,10u,0xBDA2509Bu}, // mov         dword ptr [edi+000165D4h],2
    {0x5A435Cu,0x1655Cu,0x5A435Au,6u,0x49036045u}, // mov         eax,dword ptr [edi+0001655Ch]
    {0x5A43ACu,0x1655Cu,0x5A43AAu,6u,0x4A2CDE13u}, // mov         ecx,dword ptr [ebp+0001655Ch]
    {0x5A43F6u,0x16610u,0x5A43F4u,6u,0x7BC99252u}, // mov         eax,dword ptr [ebp+00016610h]
    {0x5A4433u,0x16610u,0x5A4431u,6u,0x7BC99252u}, // mov         eax,dword ptr [ebp+00016610h]
    {0x5A44CDu,0x400u,0x5A44CBu,6u,0xAFFB7731u}, // mov         edx,dword ptr [ebp+00000400h]
    {0x5A45ABu,0x16560u,0x5A45A9u,6u,0x790299A7u}, // cmp         edi,dword ptr [ebp+00016560h]
    {0x5A45E1u,0x16610u,0x5A45DFu,6u,0x7BC99252u}, // mov         eax,dword ptr [ebp+00016610h]
    {0x5A46D0u,0x16610u,0x5A46CEu,6u,0x7BC99252u}, // mov         eax,dword ptr [ebp+00016610h]
    {0x5A4709u,0x16560u,0x5A4707u,6u,0x790299A7u}, // cmp         edi,dword ptr [ebp+00016560h]
    {0x5A471Cu,0x16610u,0x5A471Au,6u,0x7BC99252u}, // mov         eax,dword ptr [ebp+00016610h]
    {0x5A4795u,0x165D4u,0x5A4793u,6u,0x5BD7BE1Eu}, // mov         eax,dword ptr [esi+000165D4h]
    {0x5A47A4u,0x16564u,0x5A47A2u,7u,0xCF40AF6Au}, // cmp         dword ptr [esi+00016564h],0
    {0x5A47CBu,0x1661Eu,0x5A47C9u,7u,0x31E7BC36u}, // cmp         byte ptr [esi+0001661Eh],0
    {0x5A47DBu,0x16564u,0x5A47D9u,6u,0x6A3C8E58u}, // mov         dword ptr [esi+00016564h],eax
    {0x5A47F1u,0x16564u,0x5A47EFu,6u,0xF92DCC56u}, // mov         ecx,dword ptr [esi+00016564h]
    {0x5A4806u,0x16564u,0x5A4804u,6u,0xF92DCC56u}, // mov         ecx,dword ptr [esi+00016564h]
    {0x5A4816u,0x16564u,0x5A4814u,6u,0xF92DCC56u}, // mov         ecx,dword ptr [esi+00016564h]
    {0x5A4823u,0x16564u,0x5A4821u,10u,0xFA9197BAu}, // mov         dword ptr [esi+00016564h],0
    {0x5A4835u,0x16604u,0x5A4833u,6u,0x5E4E335Au}, // mov         eax,dword ptr [ecx+00016604h]
    {0x5A484Eu,0x16600u,0x5A484Cu,6u,0x2729CDBEu}, // mov         esi,dword ptr [ecx+00016600h]
    {0x5A4855u,0x165F8u,0x5A4853u,6u,0x24F45F2Bu}, // mov         edi,dword ptr [ecx+000165F8h]
    {0x5A49F6u,0x16568u,0x5A49F4u,6u,0xB72159E9u}, // mov         ebx,dword ptr [edi+00016568h]
    {0x5A4AE6u,0x165D0u,0x5A4AE4u,7u,0x419F213Du}, // cmp         dword ptr [ebp+000165D0h],0
    {0x5A4AF2u,0x165F8u,0x5A4AF0u,6u,0x5D2E3C77u}, // mov         esi,dword ptr [ebp+000165F8h]
    {0x5A4AF9u,0x16600u,0x5A4AF7u,6u,0xCF03E71Au}, // mov         edi,dword ptr [ebp+00016600h]
    {0x5A4B1Eu,0x165E8u,0x5A4B1Cu,6u,0xAD5EE8E9u}, // lea         ecx,[ebp+000165E8h]
    {0x5A4B31u,0x16564u,0x5A4B2Fu,7u,0x7DFE0549u}, // cmp         dword ptr [ebp+00016564h],0
    {0x5A4B80u,0x165E0u,0x5A4B7Eu,6u,0x1374B0B3u}, // mov         dword ptr [edi+000165E0h],eax
    {0x5A4B8Du,0x165D4u,0x5A4B8Bu,10u,0xCD1D9368u}, // mov         dword ptr [edi+000165D4h],1
    {0x5A4B97u,0x165E4u,0x5A4B95u,6u,0xA50D38AFu}, // mov         dword ptr [edi+000165E4h],esi
    {0x5A4B9Fu,0x16614u,0x5A4B9Bu,8u,0x1CC0B544u}, // movss       dword ptr [edi+00016614h],xmm0
    {0x5A4BE6u,0x16570u,0x5A4BE4u,6u,0xC1441B6Fu}, // lea         esi,[edi+00016570h]
    {0x5A4BEEu,0x1655Cu,0x5A4BECu,6u,0x7C043B9Fu}, // mov         dword ptr [edi+0001655Ch],ebx
    {0x5A4C12u,0x165C8u,0x5A4C10u,6u,0xA8EFF333u}, // cmp         dword ptr [edi+000165C8h],ebx
    {0x5A4C22u,0x165C0u,0x5A4C20u,6u,0x2602A45Fu}, // lea         esi,[edi+000165C0h]
    {0x5A4CA3u,0x16604u,0x5A4CA1u,6u,0xE9B18C5Cu}, // mov         eax,dword ptr [edi+00016604h]
    {0x5A4CBAu,0x16608u,0x5A4CB6u,8u,0xE115DCB1u}, // subss       xmm0,dword ptr [edi+00016608h]
    {0x5A4CDEu,0x1660Cu,0x5A4CDAu,8u,0x4B72C055u}, // subss       xmm1,dword ptr [edi+0001660Ch]
    {0x5A4D48u,0x1655Cu,0x5A4D46u,6u,0x1105311Du}, // mov         ecx,dword ptr [edi+0001655Ch]
    {0x5A4D8Du,0x16600u,0x5A4D8Bu,6u,0x68672C60u}, // mov         ebx,dword ptr [edi+00016600h]
    {0x5A4D93u,0x165F8u,0x5A4D91u,6u,0x24068F81u}, // mov         esi,dword ptr [edi+000165F8h]
    {0x5A4DB0u,0x165E8u,0x5A4DAEu,6u,0xA5BB4ABFu}, // lea         ecx,[edi+000165E8h]
    {0x5A4DCCu,0x1661Eu,0x5A4DCAu,7u,0xE27B247u}, // cmp         byte ptr [edi+0001661Eh],0
    {0x5A4DDEu,0x165D4u,0x5A4DDCu,6u,0x976DC2FDu}, // mov         eax,dword ptr [edi+000165D4h]
    {0x5A4DEBu,0x16614u,0x5A4DE7u,8u,0xFE4DA979u}, // movss       xmm0,dword ptr [edi+00016614h]
    {0x5A4DFBu,0x16614u,0x5A4DF7u,8u,0x1CC0B544u}, // movss       dword ptr [edi+00016614h],xmm0
    {0x5A4E17u,0x1661Du,0x5A4E15u,7u,0xC8510B66u}, // cmp         byte ptr [edi+0001661Dh],0
    {0x5A4E27u,0x165D0u,0x5A4E25u,6u,0xECE3E7CBu}, // cmp         dword ptr [edi+000165D0h],ebp
    {0x5A4E45u,0x400u,0x5A4E43u,6u,0xB8386B7Bu}, // mov         eax,dword ptr [edi+00000400h]
    {0x5A4E95u,0x1661Cu,0x5A4E93u,7u,0xA9C1D11Fu}, // mov         byte ptr [edi+0001661Ch],0
    {0x5A4E9Cu,0x16618u,0x5A4E9Au,10u,0x5803C430u}, // mov         dword ptr [edi+00016618h],0FFFFFFFFh
    {0x5A4F0Bu,0x1661Cu,0x5A4F09u,7u,0xA8C1CF8Cu}, // mov         byte ptr [edi+0001661Ch],1
    {0x5A4F12u,0x16618u,0x5A4F10u,6u,0x40C126E6u}, // mov         dword ptr [edi+00016618h],ebp
    {0x5A4F20u,0x1661Cu,0x5A4F1Eu,7u,0x4B37EABDu}, // cmp         byte ptr [edi+0001661Ch],0
    {0x5A4F29u,0x165D4u,0x5A4F27u,7u,0xBBDC243Bu}, // cmp         dword ptr [edi+000165D4h],4
    {0x5A4F58u,0x1661Cu,0x5A4F56u,7u,0x4B37EABDu}, // cmp         byte ptr [edi+0001661Ch],0
    {0x5A4F6Au,0x165D4u,0x5A4F68u,6u,0xA94E1965u}, // mov         ecx,dword ptr [edi+000165D4h]
    {0x5A4F7Eu,0x16614u,0x5A4F7Au,8u,0x9A5E6BD5u}, // subss       xmm0,dword ptr [edi+00016614h]
    {0x5A5021u,0x16560u,0x5A501Fu,6u,0xB8A466F9u}, // cmp         esi,dword ptr [edi+00016560h]
    {0x5A5045u,0x165D4u,0x5A5043u,7u,0xBBDC243Bu}, // cmp         dword ptr [edi+000165D4h],4
    {0x5A5052u,0x16618u,0x5A5050u,6u,0x40C25128u}, // cmp         ecx,dword ptr [edi+00016618h]
    {0x5A5092u,0x16618u,0x5A5090u,6u,0x40C25128u}, // cmp         ecx,dword ptr [edi+00016618h]
    {0x5A509Au,0x165D4u,0x5A5098u,7u,0xBBDC243Bu}, // cmp         dword ptr [edi+000165D4h],4
    {0x5A50D9u,0x165D4u,0x5A50D7u,6u,0x6E456B87u}, // cmp         dword ptr [edi+000165D4h],ebx
    {0x5A5120u,0x165D4u,0x5A511Eu,6u,0x6E456B87u}, // cmp         dword ptr [edi+000165D4h],ebx
    {0x5A517Au,0x804u,0x5A5178u,7u,0x66616991u}, // cmp         dword ptr [edi+00000804h],0
    {0x5A51A4u,0x165D4u,0x5A51A2u,6u,0x6E456B87u}, // cmp         dword ptr [edi+000165D4h],ebx
    {0x5A51E3u,0x804u,0x5A51E1u,6u,0x351D98A3u}, // mov         ecx,dword ptr [edi+00000804h]
    {0x5A51E9u,0x404u,0x5A51E7u,6u,0xB11297F5u}, // lea         esi,[edi+00000404h]
    {0x5A5377u,0x400u,0x5A5375u,6u,0x617CEADDu}, // mov         esi,dword ptr [ecx+00000400h]
    {0x5A539Fu,0x400u,0x5A539Du,6u,0xE6400169u}, // dec         dword ptr [ecx+00000400h]
    {0x5A5522u,0x400u,0x5A5520u,6u,0x221F764Du}, // mov         eax,dword ptr [ecx+00000400h]
    {0x5A5547u,0x400u,0x5A5545u,6u,0x1B7A3F81u}, // inc         dword ptr [ecx+00000400h]
    {0x5A555Eu,0x400u,0x5A555Cu,6u,0x1B7A3F81u}, // inc         dword ptr [ecx+00000400h]
    {0x5A57BCu,0x16600u,0x5A57BAu,6u,0x19E8A00Cu}, // mov         edi,dword ptr [ebx+00016600h]
    {0x5A57C2u,0x165F8u,0x5A57C0u,6u,0x1982845Du}, // mov         esi,dword ptr [ebx+000165F8h]
    {0x5A57DFu,0x165E8u,0x5A57DDu,6u,0xE1859757u}, // add         ebx,165E8h
    {0x5A5C61u,0x16620u,0x5A5C60u,5u,0xF743AD88u}, // push        16620h
    {0x5A5DA2u,0x165D4u,0x5A5DA0u,6u,0x7A53727u}, // mov         eax,dword ptr [ecx+000165D4h]
    {0x66B26Bu,0x16604u,0x66B269u,6u,0xB5BA4FC2u}, // mov         ecx,dword ptr [ecx+00016604h]
    {0x66DFC2u,0x16604u,0x66DFC0u,6u,0x39B60449u}, // mov         ecx,dword ptr [eax+00016604h]
    {0x66DFD4u,0x16608u,0x66DFD0u,8u,0x5430BCA0u}, // movss       xmm3,dword ptr [eax+00016608h]
    {0x66DFDCu,0x1660Cu,0x66DFD8u,8u,0xD9ACE43Cu}, // movss       xmm4,dword ptr [eax+0001660Ch]
    {0x69091Fu,0x165CCu,0x69091Du,6u,0x22FA05E8u}, // mov         eax,dword ptr [eax+000165CCh]
    {0x698E98u,0x16604u,0x698E96u,6u,0xE249E7E1u}, // mov         eax,dword ptr [eax+00016604h]
    {0x6B6F3Eu,0x16604u,0x6B6F3Cu,6u,0xE249E7E1u}, // mov         eax,dword ptr [eax+00016604h]
    {0x6B77A8u,0x16604u,0x6B77A6u,6u,0x39B60449u}, // mov         ecx,dword ptr [eax+00016604h]
    {0x6B77B0u,0x1660Cu,0x6B77ACu,8u,0x924A28D0u}, // subss       xmm2,dword ptr [eax+0001660Ch]
    {0x6B77B8u,0x16608u,0x6B77B4u,8u,0x269CE894u}, // subss       xmm0,dword ptr [eax+00016608h]
    {0x6BCD5Eu,0x16608u,0x6BCD5Au,8u,0x269CE894u}, // subss       xmm0,dword ptr [eax+00016608h]
    {0x6BCD66u,0x1660Cu,0x6BCD62u,8u,0x5C3B6A48u}, // subss       xmm1,dword ptr [eax+0001660Ch]
    {0x6BCD6Cu,0x16604u,0x6BCD6Au,6u,0xE249E7E1u}, // mov         eax,dword ptr [eax+00016604h]
    {0x6BD49Cu,0x16604u,0x6BD49Au,6u,0x4CBE6411u}, // mov         edx,dword ptr [eax+00016604h]
    {0x6BD4AFu,0x16608u,0x6BD4ADu,6u,0xACAB662Du}, // mov         ecx,dword ptr [eax+00016608h]
    {0x6BD4BBu,0x1660Cu,0x6BD4B9u,6u,0xCB5C6B89u}, // mov         edx,dword ptr [eax+0001660Ch]
    {0x751E38u,0x1661Du,0x751E36u,7u,0x807C75BFu}, // mov         byte ptr [eax+0001661Dh],0
    {0x751E9Au,0x1661Du,0x751E98u,7u,0xDD1EE3E1u}, // mov         byte ptr [ecx+0001661Dh],1
    {0x75C21Cu,0x16608u,0x75C218u,8u,0x269CE894u}, // subss       xmm0,dword ptr [eax+00016608h]
    {0x75C222u,0x16604u,0x75C220u,6u,0x39B60449u}, // mov         ecx,dword ptr [eax+00016604h]
    {0x75C22Fu,0x1660Cu,0x75C22Bu,8u,0x5C3B6A48u}, // subss       xmm1,dword ptr [eax+0001660Ch]
    {0x76882Bu,0x16604u,0x768829u,6u,0x39B60449u}, // mov         ecx,dword ptr [eax+00016604h]
    {0x7B13D6u,0x16604u,0x7B13D4u,6u,0xE249E7E1u}, // mov         eax,dword ptr [eax+00016604h]
    {0x7C49B4u,0x1661Du,0x7C49B2u,7u,0x7F7C742Cu}, // mov         byte ptr [eax+0001661Dh],1
    {0x7C4A39u,0x1661Du,0x7C4A37u,7u,0xDC1EE24Eu}, // mov         byte ptr [ecx+0001661Dh],0
    {0x7C5161u,0x1661Du,0x7C515Fu,7u,0x807C75BFu}, // mov         byte ptr [eax+0001661Dh],0
    {0x7C851Cu,0x16604u,0x7C851Au,6u,0x5E4E335Au}, // mov         eax,dword ptr [ecx+00016604h]
    {0x7C8990u,0x165C8u,0x7C898Eu,7u,0x2308D4DFu}, // cmp         dword ptr [eax+000165C8h],1
    {0x7C901Eu,0x165C8u,0x7C901Cu,7u,0x2308D4DFu}, // cmp         dword ptr [eax+000165C8h],1
    {0x7D0087u,0x16604u,0x7D0085u,6u,0x5E4E335Au}, // mov         eax,dword ptr [ecx+00016604h]
    {0x7D029Bu,0x16604u,0x7D0299u,6u,0xB5BA4FC2u}, // mov         ecx,dword ptr [ecx+00016604h]
    {0x8B1E5Du,0x165CCu,0x8B1E5Bu,6u,0x22FA05E8u}, // mov         eax,dword ptr [eax+000165CCh]
    {0x904FFDu,0x165CCu,0x904FFBu,6u,0x10A6F0D7u}, // mov         ecx,dword ptr [ecx+000165CCh]
    {0x9051CCu,0x165CCu,0x9051CAu,6u,0x22FA05E8u}, // mov         eax,dword ptr [eax+000165CCh]
    {0x90551Cu,0x165CCu,0x90551Au,6u,0x22FA05E8u}, // mov         eax,dword ptr [eax+000165CCh]
    {0x90593Cu,0x165CCu,0x90593Au,6u,0x10A6F0D7u}, // mov         ecx,dword ptr [ecx+000165CCh]
    {0x59D0C6u,0x1818u,0x59D0C4u,6u,0x50BB0DB2u}, // scheduler membership consumer
    {0x59D0DDu,0xC0Cu,0x59D0DAu,7u,0xCCC1E51Cu}, // scheduler membership consumer
    {0x59E082u,0x1810u,0x59E080u,6u,0x795DE569u}, // scheduler membership consumer
    {0x59E088u,0x180Cu,0x59E086u,6u,0xABE205AFu}, // scheduler membership consumer
    {0x59E4EEu,0x1818u,0x59E4ECu,6u,0x795DA78Cu}, // scheduler membership consumer
    {0x59E4FAu,0xC0Cu,0x59E4F7u,7u,0xEDA366D4u}, // scheduler membership consumer
    {0x5A2D1Du,0x15D5Cu,0x5A2D1Bu,6u,0xCA697457u}, // fourth scheduler pointer vector base/count
    {0x5A4364u,0x15D5Cu,0x5A4362u,6u,0xE18F7223u}, // fourth scheduler pointer vector base/count
    {0x5A43B2u,0x15D5Cu,0x5A43B0u,6u,0xE858D1F5u}, // fourth scheduler pointer vector base/count
    {0x5A4D50u,0x15D5Cu,0x5A4D4Eu,6u,0xE18F7223u}, // fourth scheduler pointer vector base/count
    {0x5A55A2u,0x800u,0x5A55A0u,6u,0x1DBA4379u}, // fourth scheduler pointer vector base/count
    {0x5A55C7u,0x800u,0x5A55C5u,6u,0xE4AA705u}, // fourth scheduler pointer vector base/count
    {0x5A55DEu,0x800u,0x5A55DCu,6u,0xE4AA705u}, // fourth scheduler pointer vector base/count
};
