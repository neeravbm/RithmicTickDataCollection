//
// Created by neera on 6/27/2025.
//

#pragma once

#include <immintrin.h>

#include <clickhouse/client.h>

#ifdef __GNUC__
#define RESTRICT __restrict__
#elif defined(_MSC_VER)
#define RESTRICT __restrict
#else
#define RESTRICT
#endif

#define APPEND_CLICKHOUSE_COLUMN(columnName, clickhouseColumnName, columnType) columnName->Reserve(columnName->Size() + block.GetRowCount()); for (unsigned int columnIndex = 0; columnIndex < block.GetColumnCount(); columnIndex++) { if (block.GetColumnName(columnIndex) == clickhouseColumnName) { columnName->Append(block[columnIndex]->As<clickhouse::Column##columnType>()); break; } }
#define APPEND_CLICKHOUSE_LOW_CARDINALITY_COLUMN(columnName, clickhouseColumnName) columnName->Reserve(columnName->Size() + block.GetRowCount()); for (unsigned int columnIndex = 0; columnIndex < block.GetColumnCount(); columnIndex++) { if (block.GetColumnName(columnIndex) == clickhouseColumnName) { columnName->Append(block[columnIndex]->As<clickhouse::ColumnLowCardinality>()); break; } }
#define APPEND_CLICKHOUSE_NULLABLE_COLUMN(columnName, clickhouseColumnName, columnType) columnName->Reserve(columnName->Size() + block.GetRowCount()); for (unsigned int columnIndex = 0; columnIndex < block.GetColumnCount(); columnIndex++) { if (block.GetColumnName(columnIndex) == clickhouseColumnName) { columnName->Append(block[columnIndex]->As<clickhouse::ColumnNullable>()->Nested()->As<clickhouse::Column##columnType>()); break; } }

#define CLICKHOUSE_APPEND_COLUMN(NAME) block.AppendColumn(#NAME, NAME);
#define CLICKHOUSE_APPEND_AS_COLUMN(NAME, COLUMN_INDEX, TYPE) { const auto& vals = std::get<COLUMN_INDEX>(results[i]); auto col = std::make_shared<clickhouse::Column##TYPE>(); col->Reserve(vals.size()); for (const auto val : vals) { col->Append(val); } NAME->AppendAsColumn(col); }
#define CLICKHOUSE_APPEND_NULLABLE_COLUMN(NAME, NULL_COLUMN_NAME) block.AppendColumn(#NAME, std::make_shared<clickhouse::ColumnNullable>(NAME, NULL_COLUMN_NAME));

namespace ClickhouseUtils {
    inline std::string DB_NAME;
    static const inline std::string MINUTELY_DATA_TABLE_NAME = "minutely_data_jan_2025";

    static const inline std::string ANNUAL_RESULTS_TABLE_NAME = "brute_force_walkforward_annual_results_20250422";
    static const inline std::string PERIOD_RESULTS_TABLE_NAME = "brute_force_walkforward_period_results";

    static const inline std::string DISTINCT_TABLE_NAME = "brute_force_walkforward_annual_results_distinct";

    static const inline std::string ENTRY_STUDY_NODES_TABLE_NAME = "study_of_studies_comparators_study_nodes";
    static const inline std::string LIMIT_OR_STOP_PRICE_STUDY_NODES_TABLE_NAME = "studies_of_studies_of_type_price_study_nodes";

    static const inline std::string DB_HOST = "192.168.0.142";
    static const inline std::string DB_USER = "default";
    static const inline std::string DB_PASSWORD = "gfxygggq";

    unsigned int getColumnIndexFromQueryResults(const clickhouse::Block& block, const std::string& clickhouseColumnName);

    template <typename SRC>
    inline void copyToFloat(const SRC* RESTRICT src, float* RESTRICT dst, size_t n) {
#if defined(__AVX512F__)
        constexpr size_t V = 16;
        size_t i = 0;
        for (; i + V <= n; i += V)
        {
            __m512i vin = _mm512_loadu_si512(src + i);
            __m512  vfp = _mm512_cvtepi32_ps(vin);

            /* use NT-store only when the address is 64-byte aligned            */
            if (((uintptr_t)(dst + i) & 63) == 0)
                _mm512_stream_ps(dst + i, vfp);
            else
                _mm512_storeu_ps(dst + i, vfp);
        }

#elif defined(__AVX2__)
        constexpr size_t V = 8;
        size_t i = 0;
        for (; i + V <= n; i += V)
        {
            __m256i vin = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + i));
            __m256  vfp = _mm256_cvtepi32_ps(vin);

            /* 32-byte alignment check */
            if (((uintptr_t)(dst + i) & 31) == 0)
                _mm256_stream_ps(dst + i, vfp);
            else
                _mm256_storeu_ps(dst + i, vfp);
        }
#else
        size_t i = 0; // scalar path
#endif

        /* tail */
        for (size_t j = i; j < n; ++j)
            dst[j] = static_cast<float>(src[j]);
    }

    void append(const clickhouse::Block& block, const unsigned int nRows, const unsigned int pos, auto& dst) {
        using DstCol = typename std::remove_reference_t<decltype(dst)>::element_type;
        dst->Reserve(dst->Size() + nRows);

        /*  Low-Cardinality path  ───────────────────────────────────────── */
        if constexpr (std::is_base_of_v<clickhouse::ColumnLowCardinality, DstCol>) {
            if (auto srcLC = block[pos]->As<clickhouse::ColumnLowCardinality>()) {
                dst->Append(srcLC); // still the quickest way
                return;
            }

            /* --------------------- slow fall-back ----------------------- */
            auto src = block[pos]; // any column kind
            for (size_t r = 0; r < nRows; ++r) {
                /* for LC(String…) dictionaries the public Append takes
                   either std::string  or  std::string_view                */
                const auto str = src->GetItem(r).get<std::string>();
                dst->Append(str); //   ←  public API
            }
        }
        /* ────────────────────────────────────────────────────────────────
   FAST PATH 2 : destination is Nullable
   ──────────────────────────────────────────────────────────────── */
        else if constexpr (std::is_base_of_v<clickhouse::ColumnNullable, DstCol>) {
            using NestedCol = typename DstCol::NestedColumnType; // e.g. ColumnUInt16
            auto dstNullCol = dst.get();

            /* Source is nullable as well --------------------------------- */
            if (auto srcNullCol = block[pos]->As<clickhouse::ColumnNullable>()) {
                const auto* nullmap = srcNullCol->Nulls()
                                                ->As<clickhouse::ColumnUInt8>()
                                                ->GetWritableData()
                                                .data(); // read-only use

                auto srcNested = srcNullCol->Nested()->As<NestedCol>();

                for (size_t r = 0; r < nRows; ++r) {
                    if (nullmap[r])
                        // insert a NULL
                        dstNullCol->Append(typename DstCol::ValueType{});
                    else
                        // insert the real value
                        dstNullCol->Append(typename DstCol::ValueType{ srcNested->At(r) }); // optional with payload
                }
                return;
            }

            /* Source is *not* nullable ----------------------------------- */
            if (auto srcPlain = block[pos]->As<NestedCol>()) {
                dstNullCol->Append(srcPlain); // driver overload: append everything
                return;
            }

            /* Any other combination is a programmer error. */
            throw std::runtime_error("append(): incompatible source column for nullable destination");
        }
        /*  Ordinary columns  ───────────────────────────────────────────── */
        else if (auto srcPlain = block[pos]->As<DstCol>()) {
            dst->Append(srcPlain);
        }
        else {
            throw std::runtime_error("append(): unhandled source/destination types");
        }
    };

    /**
* Append one std::vector as a single row to a ColumnArray.
*
* @tparam DestT  element type stored in the ColumnArray    (e.g. uint16_t)
* @tparam SrcT   element type of the std::vector           (e.g. unsigned int)
*
* Example:
*     appendVectorAsRow<uint16_t>(arrayCol, unsignedIntVec);
*/
    template <typename DestT,
              typename SrcT,
              std::enable_if_t<std::is_arithmetic_v<DestT> &&
                               std::is_arithmetic_v<SrcT>, int>  = 0>
    void appendVectorAsRow(const std::shared_ptr<clickhouse::ColumnArray>& arr,
                           const std::vector<SrcT>& row) {
        using DestCol = clickhouse::ColumnVector<DestT>; // ColumnUInt16, ColumnInt32, …

        /* 1. Overflow / narrowing guard (only triggers when DestT is narrower). */
        if constexpr (sizeof(SrcT) > sizeof(DestT)) {
            const auto hi = std::numeric_limits<DestT>::max();
            const auto lo = std::numeric_limits<DestT>::lowest();
            for (SrcT v : row)
                if (v > hi || v < lo)
                    throw std::overflow_error("appendVectorAsRow: value out of range");
        }

        /* 2. Build a temporary scalar column of the correct element type. */
        auto elemCol = std::make_shared<DestCol>();
        elemCol->Reserve(row.size());
        for (SrcT v : row)
            elemCol->Append(static_cast<DestT>(v));

        /* 3. Append that column as **one row** to the array. */
        arr->AppendAsColumn(elemCol);
    }

    /*───────────────────────────────────────────────────────────────────────────
0.  Helpers
──────────────────────────────────────────────────────────────────────────*/
    /*───────────────────────────────────────────────────────────────────────────
      1.  Primary template – defaults to false
     ──────────────────────────────────────────────────────────────────────────*/
    template <class, class = void>
    struct is_column_ptr : std::false_type {};

    /*───────────────────────────────────────────────────────────────────────────
      2.  Partial specialisation for std::shared_ptr<U>
          (substitution succeeds only if T is exactly shared_ptr<Something>)
     ──────────────────────────────────────────────────────────────────────────*/
    template <class U>
    struct is_column_ptr<std::shared_ptr<U>,
                         std::enable_if_t<std::is_base_of_v<clickhouse::Column, U>>>
        : std::true_type {};

    /* helper variable template */
    template <class T>
    inline constexpr bool is_column_ptr_v = is_column_ptr<std::decay_t<T>>::value;

    // 0-b)  Is a std::vector ?
    template <class T>
    struct is_std_vector : std::false_type {};

    template <class U, class A>
    struct is_std_vector<std::vector<U, A>> : std::true_type {};

    template <class T>
    inline constexpr bool is_vector_v = is_std_vector<std::decay_t<T>>::value;

    // 0-c)  Map scalar → ClickHouse column type
    template <class T>
    struct ColumnFor; // primary = undefined
    template <>
    struct ColumnFor<std::uint8_t> {
        using type = clickhouse::ColumnUInt8;
    };

    template <>
    struct ColumnFor<std::uint16_t> {
        using type = clickhouse::ColumnUInt16;
    };

    template <>
    struct ColumnFor<std::uint32_t> {
        using type = clickhouse::ColumnUInt32;
    };

    template <>
    struct ColumnFor<float> {
        using type = clickhouse::ColumnFloat32;
    };

    template <>
    struct ColumnFor<double> {
        using type = clickhouse::ColumnFloat64;
    };

    template <>
    struct ColumnFor<std::string> {
        using type = clickhouse::ColumnString;
    };

    template <class T>
    using ColumnForT = typename ColumnFor<std::decay_t<T>>::type;

    // extra trait: “is a pair of column pointers?”
    template <class P>
    struct is_col_ptr_pair : std::false_type {};

    template <class A, class B>
    struct is_col_ptr_pair<std::pair<A, B>>
        : std::bool_constant<is_column_ptr_v<A> && is_column_ptr_v<B>> {};

    template <class P>
    inline constexpr bool is_col_ptr_pair_v = is_col_ptr_pair<std::decay_t<P>>::value;

    template <class DesiredCol = void, class Arg>
    void appendColumnToBlock(clickhouse::Block& block,
                             std::string_view name,
                             Arg&& arg) {
        using A = std::decay_t<Arg>;

        /*--------------------------------------------------------------*
 |  Path 1: user passed a prepared clickhouse::Column (pointer) |
 *--------------------------------------------------------------*/
        if constexpr (is_column_ptr_v<A>) {
            block.AppendColumn(std::string{name}, std::forward<Arg>(arg));
        }

        /* Path-2 : std::vector<Scalar> → choose DesiredCol or deduce */
        else if constexpr (is_vector_v<A>) {
            using ValT = typename A::value_type;
            using ColT = std::conditional_t<
                std::is_same_v<DesiredCol, void>,
                ColumnForT<ValT>, // default mapping
                DesiredCol>; // caller override

            auto col = std::make_shared<ColT>();
            col->Reserve(arg.size());
            for (const auto& v : arg) col->Append(v);
            block.AppendColumn(name, col);
        }

        /* ───── path 3: caller passes {dataCol, nullMaskCol} ─────────────── */
        else if constexpr (is_col_ptr_pair_v<A>) {
            block.AppendColumn(
                std::string{name},
                std::make_shared<clickhouse::ColumnNullable>(
                    arg.first, // data column
                    arg.second) // null-mask column
            );
        }

        /* ───── anything else == error at compile-time ───────────────────── */
        else {
            static_assert(!sizeof(A),
                          "appendColumnAuto(): unsupported argument -- need a ClickHouse "
                          "column ptr, a std::vector<Scalar>, or a pair<ptr,ptr> for Nullable");
        }
    }

    template <class Wrapped>
    auto* unwrapNullable(clickhouse::ColumnRef& col) {
        if (auto* n = col->As<clickhouse::ColumnNullableT<Wrapped>>().get())
            return n->Nested()->template As<Wrapped>().get();
        return col->As<Wrapped>().get(); // non-nullable column of the same base type
    }

    /* ------------------------------------------------------------------ */
    /*  copyNumericNullableToFloat()                                       */
    /*     • Works for ColumnNullableT<Wrapped>, where Wrapped is a        */
    /*       numeric ClickHouse column type (UInt8, UInt16, …, Float32).   */
    /*     • Fills dst[0 … nRows-1].                                       */
    /*     • For NULL rows writes quiet_NaN().                             */
    /* ------------------------------------------------------------------ */
    template <class Wrapped>
    void copyNumericNullableToFloat(const clickhouse::ColumnRef& col, float* dst, size_t nRows) {
        static_assert(std::is_base_of_v<clickhouse::Column, Wrapped>, "Wrapped must be a ClickHouse column type");

        if (auto* ncol = col->As<clickhouse::ColumnNullableT<Wrapped>>().get()) {
            const auto* nullmap = ncol->Nulls() // ColumnRef → ColumnUInt8
                                      ->template As<clickhouse::ColumnUInt8>() // down-cast
                                      ->GetWritableData() // returns const std::vector<UInt8>&
                                      .data();
            const auto* data = ncol->Nested()->template As<Wrapped>()->GetWritableData().data(); // raw values

            const float NaN = std::numeric_limits<float>::quiet_NaN();

            for (size_t i = 0; i < nRows; ++i)
                dst[i] = nullmap[i] ? NaN : static_cast<float>(data[i]);
        }
        else if (auto* ncol_base = col->As<clickhouse::ColumnNullable>().get()) {
            // make sure the nested column is the expected numeric type
            if (auto* data_col = ncol_base->Nested()->As<Wrapped>().get()) {
                const auto* nullmap = ncol_base->Nulls()
                                               ->As<clickhouse::ColumnUInt8>()
                                               ->GetWritableData().data(); // read-only use is fine

                const auto* data = data_col->GetWritableData().data();
                const float NaN = std::numeric_limits<float>::quiet_NaN();

                for (size_t i = 0; i < nRows; ++i)
                    dst[i] = nullmap[i] ? NaN : static_cast<float>(data[i]);
            }
            else {
                throw std::runtime_error("Unexpected nested type");
            }
        }
        else /* non-nullable column -------------------------------------- */
            if (auto* colPlain = col->As<Wrapped>().get()) {
                copyToFloat(colPlain->GetWritableData().data(), dst, nRows);
            }
            else {
                throw std::runtime_error("Unexpected column type in copyNumericNullableToFloat");
            }
    }
}
