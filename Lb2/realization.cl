kernel void list_prefix_sum(global float* src, uint n)
{
    local float lmem[TILE]; 
    uint x = get_global_id(0);
    uint lx = get_local_id(0);
    lmem[lx] = src[x]; 

    barrier(CLK_LOCAL_MEM_FENCE);
    
    if (lx == 0)
    {
        for (int i = 1; i < n; i++) 
        {
            lmem[i] += lmem[i - 1];
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE);
    src[x] = lmem[lx];
}

kernel void part1(global float* src, global float* part)
{
    uint x = get_global_id(0);
    uint lx = get_local_id(0);
    uint gx = get_group_id(0);

    local float lmem[TILE];
    lmem[lx] = src[x];

    barrier(CLK_LOCAL_MEM_FENCE);
    if (lx == 0)
    {
        for (int i = 1; i < TILE; i++)
        {
            lmem[i] += lmem[i - 1];
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE);
    src[x] = lmem[lx];

    //part[gx] = lmem[get_local_size(0) - 1];

    if (lx == TILE - 1)
    {
        part[gx] = lmem[lx];
    }
}

#if VECTOR == 16

    #define vloadN vload16
    #define vstoreN vstore16
    #define vector float16

#elif VECTOR == 8
    #define vloadN vload8
    #define vstoreN vstore8
    #define vector float8 

#elif VECTOR == 4
    #define vloadN vload4
    #define vstoreN vstore4
    #define vector float4
#endif




kernel void part2(global float* src, global float* part)
{
    uint x = get_global_id(0);
    vector tmp; 
    for (int i = 0; i < VECTOR_AMOUNT; i++)
    {
        tmp = vloadN(0, src + x * TILE + (i * VECTOR));
        tmp += part[x];
        vstoreN(tmp, 0, src + x * TILE + (i * VECTOR));
    }
}
 
