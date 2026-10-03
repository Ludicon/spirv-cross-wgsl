struct EmptyStructTest
{
    empty_struct_member : i32,
}

fn GetValue(_self : EmptyStructTest) -> f32
{
    return 0.0f;
}

fn GetValue_4(_self : EmptyStructTest) -> f32
{
    return 0.0f;
}

fn frag_main()
{
    var _24 : EmptyStructTest = EmptyStructTest(0);
    var emptyStruct : EmptyStructTest;
    var value : f32 = GetValue(emptyStruct);
    value = GetValue_4(_24);
}

@fragment
fn main()
{
    frag_main();
}
