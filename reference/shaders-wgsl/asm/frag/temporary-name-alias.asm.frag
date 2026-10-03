fn frag_main()
{
    let constituent = f32(0);
    var _mat3 = mat3x3f(vec3f(constituent), vec3f(constituent), vec3f(constituent));
    let constituent_1 = f32(1);
    _mat3 = mat3x3f(vec3f(constituent_1), vec3f(constituent_1), vec3f(constituent_1));
}

@fragment
fn main()
{
    frag_main();
}
