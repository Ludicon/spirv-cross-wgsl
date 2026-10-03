fn frag_main()
{
    var sw : i32 = 42;
    var result : i32 = 0;
    switch (sw)
    {
        case -42:
        {
            result = 42;
            result = 420;
            result = 420;
            break;
        }
        case 420:
        {
            result = 420;
            result = 420;
            break;
        }
        case -1234:
        {
            result = 420;
            break;
        }
        default:
        {
            break;
        }
    }
}

@fragment
fn main()
{
    frag_main();
}
