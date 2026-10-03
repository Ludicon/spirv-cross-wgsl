fn frag_main()
{
    var v = vec3f(0.0f);
    if (false)
    {
        v.x = 99.0f;
        v.x = 88.0f;
    }
}

@fragment
fn main()
{
    frag_main();
}
