package edu.washington.cs.diamond;

import org.bytedeco.javacpp.*;
import org.bytedeco.javacpp.annotation.*;

//@Platform(include={"<vector>", "<string>"})
@Platform(include={"data_types.h"})

public class DiamondUtil {

    /*
     * This version of the StringVector class was copied from the file
     * "opencv_core.java" in javacv on Google Code:
     * https://code.google.com/p/javacv
     */
    @Name("std::vector<std::string>")
    public static class StringVector extends Pointer {
        static { Loader.load(); }
        public StringVector(String ... array) { this(array.length); put(array); }
        public StringVector()       { allocate();  }
        public StringVector(long n) { allocate(n); }
        public StringVector(Pointer p) { super(p); }
        private native void allocate();
        private native void allocate(@Cast("size_t") long n);

        public native long size();
        public native void resize(@Cast("size_t") long n);

        @Index @ByRef public native String get(@Cast("size_t") long i);
        public native StringVector put(@Cast("size_t") long i, String value);

        public StringVector put(String ... array) {
            if (size() < array.length) { resize(array.length); }
            for (int i = 0; i < array.length; i++) {
                put(i, array[i]);
            }
            return this;
        }
    }

    @Name("std::vector<uint64_t>")
    public static class LongVector extends Pointer {
        static { Loader.load(); }
        public LongVector(long ... array) { this(array.length); put(array); }
        public LongVector()       { allocate();  }
        public LongVector(long n) { allocate(n); }
        public LongVector(Pointer p) { super(p); }
        private native void allocate();
        private native void allocate(@Cast("size_t") long n);

        public native long size();
        public native void resize(@Cast("size_t") long n);

        @Index public native long get(@Cast("size_t") long i);
        public native LongVector put(@Cast("size_t") long i, long value);

        public LongVector put(long ... array) {
            if (size() < array.length) { resize(array.length); }
            for (int i = 0; i < array.length; i++) {
                put(i, array[i]);
            }
            return this;
        }
    }

    @Name("std::vector<diamond::DObject *>") public static class DObjectVector extends Pointer {
        static { Loader.load(); }
        /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
        public DObjectVector(Pointer p) { super(p); }
        public DObjectVector(Diamond.DObject ... array) { this(array.length); put(array); }
        public DObjectVector()       { allocate();  }
        public DObjectVector(long n) { allocate(n); }
        private native void allocate();
        private native void allocate(@Cast("size_t") long n);
        public native @Name("operator=") @ByRef DObjectVector put(@ByRef DObjectVector x);

        public native long size();
        public native void resize(@Cast("size_t") long n);

        @Index public native Diamond.DObject get(@Cast("size_t") long i);
        public native void put(@Cast("size_t") long i, Diamond.DObject value);

        public DObjectVector put(Diamond.DObject ... array) {
            if (size() != array.length) { resize(array.length); }
            for (int i = 0; i < array.length; i++) {
                put(i, array[i]);
            }
            return this;
        }
    }

    /*
     * This version of the StringVector class was copied from the file "videoInputLib.java" in javacpp-presets
     * on Github: https://github.com/bytedeco/javacpp-presets
     */
//    @Name("std::vector<std::string>") public static class StringVector extends Pointer {
//        static { Loader.load(); }
//        /** Pointer cast constructor. Invokes {@link Pointer#Pointer(Pointer)}. */
//        public StringVector(Pointer p) { super(p); }
//        public StringVector(BytePointer ... array) { this(array.length); put(array); }
//        public StringVector()       { allocate();  }
//        public StringVector(long n) { allocate(n); }
//        private native void allocate();
//        private native void allocate(@Cast("size_t") long n);
//        public native @Name("operator=") @ByRef StringVector put(@ByRef StringVector x);
//
//        public native long size();
//        public native void resize(@Cast("size_t") long n);
//
//        @Index public native @StdString BytePointer get(@Cast("size_t") long i);
//        public native StringVector put(@Cast("size_t") long i, @StdString BytePointer value);
//
//        public StringVector put(BytePointer ... array) {
//            if (size() != array.length) { resize(array.length); }
//            for (int i = 0; i < array.length; i++) {
//                put(i, array[i]);
//            }
//            return this;
//        }
//    }
}
