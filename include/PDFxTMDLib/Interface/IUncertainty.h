#pragma once
#include <PDFxTMDLib/Common/Uncertainty.h>
#include <PDFxTMDLib/Common/YamlMetaInfo/YamlErrorInfo.h>
#include <vector>

namespace PDFxTMD
{
class IUncertainty
{
  public:
    template <typename UncertaintyApproachT>
    explicit IUncertainty(UncertaintyApproachT uncertaintyApproach)
        : pimpl_(new OwningModel<UncertaintyApproachT>(std::move(uncertaintyApproach)),
                 [](void *uncertaintyApproachBytes) {
                     using Model = OwningModel<UncertaintyApproachT>;
                     auto *const model = static_cast<Model *>(uncertaintyApproachBytes);
                     delete model;
                 }),
          uncertaintyOperation_([](void *uncertaintyApproachBytes,
                                   const std::vector<double> &values, const int numCoreErrMember,
                                   const double cl, PDFUncertainty &uncertainty) -> void {
              using Model = OwningModel<UncertaintyApproachT>;
              auto *const model = static_cast<Model *>(uncertaintyApproachBytes);
              return model->Uncertainty(values, numCoreErrMember, cl, uncertainty);
          }),
          correlationOperation_(
              [](void *uncertaintyApproachBytes, const std::vector<double> &valuesA,
                 const std::vector<double> &valuesB, const int numCoreErrMember) -> double {
                  using Model = OwningModel<UncertaintyApproachT>;
                  auto *const model = static_cast<Model *>(uncertaintyApproachBytes);
                  return model->Correlation(valuesA, valuesB, numCoreErrMember);
              }),
          clone_([](void *uncertaintyApproachBytes) -> void * {
              using Model = OwningModel<UncertaintyApproachT>;
              auto *const model = static_cast<Model *>(uncertaintyApproachBytes);
              return new Model(*model);
          })
    {
    }

    void Uncertainty(const std::vector<double> &values, const int numCoreErrMember, const double cl,
                     PDFUncertainty &uncertainty) const
    {
        uncertaintyOperation_(pimpl_.get(), values, numCoreErrMember, cl, uncertainty);
    }

    double Correlation(const std::vector<double> &valuesA, const std::vector<double> &valuesB,
                       const int numCoreErrMember) const
    {
        return correlationOperation_(pimpl_.get(), valuesA, valuesB, numCoreErrMember);
    }
    /**
     * @brief Copy constructor for ICPDF objects.
     *
     * This constructor creates a new ICPDF object as a copy of another ICPDF object.
     *
     * @param other The ICPDF object to copy.
     */
    IUncertainty(const IUncertainty &other)
        : pimpl_(other.clone_(other.pimpl_.get()), other.pimpl_.get_deleter()),
          clone_(other.clone_), correlationOperation_(other.correlationOperation_),
          uncertaintyOperation_(other.uncertaintyOperation_)
    {
    }

    /**
     * @brief Assignment operator for ICPDF objects.
     *
     * This operator assigns the value of another ICPDF object to the current ICPDF object.
     *
     * @param other The ICPDF object to assign from.
     * @return A reference to the current ICPDF object.
     */
    IUncertainty &operator=(IUncertainty const &other)
    {
        using std::swap;
        IUncertainty copy(other);
        swap(pimpl_, copy.pimpl_);
        swap(clone_, copy.clone_);
        swap(correlationOperation_, copy.correlationOperation_);
        swap(uncertaintyOperation_, copy.uncertaintyOperation_);
        return *this;
    }

    IUncertainty(IUncertainty &&other) noexcept = default;
    ~IUncertainty() = default;
    IUncertainty &operator=(IUncertainty &&other) = default;

  private:
    template <typename UncertaintyApproachT> struct OwningModel
    {
        OwningModel(UncertaintyApproachT uncertaintyApporach)
            : uncertaintyApporach_(std::move(uncertaintyApporach))
        {
        }

        double Correlation(const std::vector<double> &valuesA, const std::vector<double> &valuesB,
                           const int numCoreErrMember)
        {
            return uncertaintyApporach_.Correlation(valuesA, valuesB, numCoreErrMember);
        }
        void Uncertainty(const std::vector<double> &values, const int numCoreErrMember,
                         const double cl, PDFUncertainty &uncertainty)
        {
            return uncertaintyApporach_.Uncertainty(values, numCoreErrMember, cl, uncertainty);
        }
        UncertaintyApproachT uncertaintyApporach_;
    };

    using DestroyOperation = void(void *);
    using CloneOperation = void *(void *);
    using UncertaintyOperation = void(void *, const std::vector<double> &, const int, const double,
                                      PDFUncertainty &);
    using CorrelationOperation = double(void *, const std::vector<double> &,
                                        const std::vector<double> &, const int);

    std::unique_ptr<void, DestroyOperation *> pimpl_;
    CloneOperation *clone_{nullptr};
    UncertaintyOperation *uncertaintyOperation_{nullptr};
    CorrelationOperation *correlationOperation_{nullptr};
};
} // namespace PDFxTMD
