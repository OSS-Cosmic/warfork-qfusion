#include "r_renderer_api.h"

R_BindFrameBufferObjectFn R_BindFrameBufferObject;
R_Set2DModeFn R_Set2DMode;
R_SetGammaFn R_SetGamma;
R_ScissorFn R_Scissor;
R_GetScissorFn R_GetScissor;
R_ResetScissorFn R_ResetScissor;
R_DrawStretchQuickFn R_DrawStretchQuick;
R_ClearRefInstStackFn R_ClearRefInstStack;
R_PushRefInstFn R_PushRefInst;
R_PopRefInstFn R_PopRefInst;
R_DeferDataSyncFn R_DeferDataSync;
R_DataSyncFn R_DataSync;
R_FinishFn R_Finish;
R_FlushFn R_Flush;
