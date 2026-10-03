override _18 : u32 = 3u;

fn frag_main()
{
    var v : vec3f = vec3f(0.0f);
    if (false)
    {
        v[0] = 99.0f;
    }
}

@fragment
fn main()
{
    frag_main();
}
