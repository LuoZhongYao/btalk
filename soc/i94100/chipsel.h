/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/07/04
 ************************************************/
#ifndef __CHIPSEL_H__
#define __CHIPSEL_H__

#include <config.h>

#if	defined(M480)

# include <NuMicro.h>

# define __PDMA_GET_INT_STATUS(pdma) PDMA_GET_INT_STATUS(pdma)
# define __PDMA_CLR_TD_FLAG(pdma, mask) PDMA_CLR_TD_FLAG(pdma, mask)
# define __PDMA_GET_TD_STS(pdma)       PDMA_GET_TD_STS(pdma)
# define __PDMA_CLR_TMOUT_FLAG(pdma, ...) PDMA_CLR_TMOUT_FLAG(pdma, __VA_ARGS__)

#elif defined(I94100)

# include <Platform.h>

# define PDMA_Open(pdma, chmask) PDMA_Open(chmask)
# define PDMA_SetTransferCnt(pdma, ...) PDMA_SetTransferCnt(__VA_ARGS__)
# define PDMA_SetTransferAddr(pdma, ...) PDMA_SetTransferAddr(__VA_ARGS__)
# define PDMA_SetTransferMode(pdma, ...) PDMA_SetTransferMode(__VA_ARGS__)
# define PDMA_SetBurstType(pdma, ...) PDMA_SetBurstType(__VA_ARGS__)
# define PDMA_SetTimeOut(pdma, ...) PDMA_SetTimeOut(__VA_ARGS__)
# define PDMA_EnableInt(pdma, ...) PDMA_EnableInt(__VA_ARGS__)
# define PDMA_EnableTimeout(pdma, ...) PDMA_EnableTimeout(__VA_ARGS__)
# define __PDMA_GET_INT_STATUS(pdma) PDMA_GET_INT_STATUS()
# define __PDMA_CLR_TD_FLAG(pdma, mask) PDMA_CLR_TD_FLAG(mask)
# define __PDMA_GET_TD_STS(pdma)       PDMA_GET_TD_STS()
# define __PDMA_CLR_TMOUT_FLAG(pdma, ...) PDMA_CLR_TMOUT_FLAG(__VA_ARGS__)
# define UART_SetLineConfig(...) UART_SetLine_Config(__VA_ARGS__)

#endif

#endif /* __CHIPSEL_H__*/

