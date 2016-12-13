using System;
using System.Collections.Generic;
using System.Net.Configuration;

namespace ATLASharp
{
    public class Context
    {
    }

    public abstract class Term
    {
        public Term Shift(int offset)
        {
            return Shift(offset, 0);
        }

        public Term Substitute(int from, Term to)
        {
            return Substitute(from, to, 0);
        }

        public Term SubstituteWith(Term term)
        {
            return Substitute(0, term.Shift(1)).Shift(-1);
        }

        public Term EvalOnce(Context ctx)
        {
            return DoEvalOnce(ctx);
        }

        public Term Eval(Context ctx = null)
        {
            Term result;
            var result2 = this;
            do
            {
                result = result2;
                result2 = result.EvalOnce(ctx);
            } while (!ReferenceEquals(result, result2));
            return result;
        }

        public static Application operator *(Term left, Term right)
        {
            return new Application(left, right);
        }

        public static Abstraction operator --(Term body)
        {
            return new Abstraction(body);
        }

        public abstract override string ToString();

        internal abstract Term Shift(int offset, int curr);

        internal abstract Term Substitute(int from, Term to, int curr);

        internal abstract Term DoEvalOnce(Context ctx);
    }

    public class Variable : Term
    {
        public int Index { get; }

        public Variable(int index)
        {
            Index = index;
        }

        public override string ToString()
        {
            return Index.ToString();
        }

        internal override Term Shift(int offset, int curr)
        {
            if (Index >= curr)
            {
                return new Variable(Index + offset);
            }
            return this;
        }

        internal override Term Substitute(int from, Term to, int curr)
        {
            if (Index == from + curr)
            {
                return to.Shift(curr);
            }
            return this;
        }

        internal override Term DoEvalOnce(Context ctx)
        {
            return this;
        }
    }

    public class Abstraction : Term
    {
        public Term Body { get; }

        public Abstraction(Term body)
        {
            Body = body;
        }

        public override string ToString()
        {
            return "(\\." + Body + ")";
        }

        internal override Term Shift(int offset, int curr)
        {
            var shiftedBody = Body.Shift(offset, curr + 1);
            if (ReferenceEquals(Body, shiftedBody))
            {
                return this;
            }
            return new Abstraction(shiftedBody);
        }

        internal override Term Substitute(int from, Term to, int curr)
        {
            var substitutedBody = Body.Substitute(from, to, curr + 1);
            if (ReferenceEquals(substitutedBody, Body))
            {
                return this;
            }
            return new Abstraction(substitutedBody);
        }

        internal override Term DoEvalOnce(Context ctx)
        {
            var result = Body.DoEvalOnce(ctx);
            if (result == Body)
            {
                return this;
            }
            return new Abstraction(result);
        }
    }

    public class Application : Term
    {
        public Term Left { get; }

        public Term Right { get; }

        public Application(Term left, Term right)
        {
            Left = left;
            Right = right;
        }

        public override string ToString()
        {
            return "(" + Left + " " + Right + ")";
        }

        internal override Term Shift(int offset, int curr)
        {
            var shiftedLeft = Left.Shift(offset, curr);
            var shiftedRight = Right.Shift(offset, curr);

            if (ReferenceEquals(Left, shiftedLeft) && ReferenceEquals(Right, shiftedRight))
            {
                return this;
            }
            return new Application(shiftedLeft, shiftedRight);
        }

        internal override Term Substitute(int from, Term to, int curr)
        {
            var substitutedLeft = Left.Substitute(from, to, curr);
            var substitutedRight = Right.Substitute(from, to, curr);

            if (ReferenceEquals(Left, substitutedLeft) && ReferenceEquals(Right, substitutedRight))
            {
                return this;
            }
            return new Application(substitutedLeft, substitutedRight);
        }

        internal override Term DoEvalOnce(Context ctx)
        {
            if (Left is Abstraction && (Right is Abstraction || Right is Variable))
            {
                return ((Abstraction)Left).Body.SubstituteWith(Right);
            }
            if (Left is Abstraction || Left is Variable)
            {
                var eRight = Right.DoEvalOnce(ctx);
                if (ReferenceEquals(Right, eRight))
                {
                    return this;
                }
                return new Application(Left, eRight);
            }
            var evalLeft = Left.DoEvalOnce(ctx);
            if (ReferenceEquals(Left, evalLeft))
            {
                return this;
            }
            return new Application(evalLeft, Right);
        }
    }

    public static class Operation
    {
        public static Variable Var(int index)
        {
            return new Variable(index);
        }

        public static Application App(Term left, Term right)
        {
            return new Application(left, right);
        }

        public static Abstraction Abs(Term body)
        {
            return new Abstraction(body);
        }

        public static Abstraction Abs(this int a, Term body)
        {
            return new Abstraction(body);
        }
    }
}
