struct B
{
    a : f32,
    b : f32,
}

struct UBO
{
    some_value : i32,
}

@group(0) @binding(0) var<uniform> _51 : UBO;

fn partial_inout(x : ptr<function, vec4f>)
{
    (*x).x = 10.0f;
}

fn complete_inout(x : ptr<function, vec4f>)
{
    (*x) = vec4f(50.0f);
}

fn branchy_inout(v : ptr<function, vec4f>)
{
    (*v).y = 20.0f;
    if (_51.some_value == 20)
    {
        (*v) = vec4f(50.0f);
    }
}

fn branchy_inout_2(v : ptr<function, vec4f>)
{
    if (_51.some_value == 20)
    {
        (*v) = vec4f(50.0f);
    }
    else
    {
        (*v) = vec4f(70.0f);
    }
    (*v).y = 20.0f;
}

fn partial_inout_17(b : ptr<function, B>)
{
    (*b).b = 40.0f;
}

fn complete_inout_35(b : ptr<function, B>)
{
    (*b) = B(100.0f, 200.0f);
}

fn branchy_inout_23(b : ptr<function, B>)
{
    (*b).b = 20.0f;
    if (_51.some_value == 20)
    {
        (*b) = B(10.0f, 40.0f);
    }
}

fn branchy_inout_2_29(b : ptr<function, B>)
{
    if (_51.some_value == 20)
    {
        (*b) = B(10.0f, 40.0f);
    }
    else
    {
        (*b) = B(70.0f, 70.0f);
    }
    (*b).b = 20.0f;
}

fn frag_main()
{
    var a : vec4f = vec4f(10.0f);
    var param : vec4f = a;
    partial_inout(&param);
    a = param;
    var param_1 : vec4f;
    complete_inout(&param_1);
    a = param_1;
    var param_2 : vec4f = a;
    branchy_inout(&param_2);
    a = param_2;
    var param_3 : vec4f;
    branchy_inout_2(&param_3);
    a = param_3;
    var b : B = B(10.0f, 20.0f);
    var param_4 : B = b;
    partial_inout_17(&param_4);
    b = param_4;
    var param_5 : B;
    complete_inout_35(&param_5);
    b = param_5;
    var param_6 : B = b;
    branchy_inout_23(&param_6);
    b = param_6;
    var param_7 : B;
    branchy_inout_2_29(&param_7);
    b = param_7;
}

@fragment
fn main()
{
    frag_main();
}
