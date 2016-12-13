using System;

using static ATLASharp.Operation;

namespace ATLASharp
{
    class Program
    {
        static void Main(string[] args)
        {
            var zero = Abs(Abs(Var(0)));
            var succ = Abs(Abs(Abs(Var(1) * (Var(2) * Var(1) * Var(0)))));
            Console.WriteLine($"zero: {zero}");
            Console.WriteLine($"succ: {succ}");
            Console.WriteLine($"one:  {(succ*zero).Eval()}");
        }
    }
}
