#pragma once

#include "PDFxTMDLib/Common/PartonUtils.h"

#include <memory>
#include <type_traits>
#include <utility>

namespace PDFxTMD
{

/**
 * @brief Type-erased interface for collinear double parton distribution functions.
 *
 * This class wraps any DPD implementation that provides:
 *
 *   double dpd(PartonFlavor flavor1,
 *               PartonFlavor flavor2,
 *               double x1,
 *               double mu1_2,
 *               double x2,
 *               double mu2_2);
 *
 * For example, it can wrap:
 *
 *   PDFxTMD::CollinearDPD
 */
class ICDPD
{
  public:
    template <typename DPDApproachT,
              typename = std::enable_if_t<!std::is_same_v<std::decay_t<DPDApproachT>, ICDPD>>>
    explicit ICDPD(DPDApproachT &&dpdApproach)
        : pimpl_(
              new OwningModel<std::decay_t<DPDApproachT>>(std::forward<DPDApproachT>(dpdApproach)),
              [](void *dpdApproachBytes) {
                  using Model = OwningModel<std::decay_t<DPDApproachT>>;
                  auto *const model = static_cast<Model *>(dpdApproachBytes);
                  delete model;
              }),
          dpdOperation_([](void *dpdApproachBytes, PartonFlavor flavor1, PartonFlavor flavor2,
                           double x1, double mu1_2, double x2, double mu2_2) -> double {
              using Model = OwningModel<std::decay_t<DPDApproachT>>;
              auto *const model = static_cast<Model *>(dpdApproachBytes);

              return model->dpd(flavor1, flavor2, x1, mu1_2, x2, mu2_2);
          }),
          clone_([](void *dpdApproachBytes) -> void * {
              using Model = OwningModel<std::decay_t<DPDApproachT>>;
              auto *const model = static_cast<Model *>(dpdApproachBytes);
              return new Model(*model);
          })
    {
    }

    /**
     * @brief Evaluate the DPD for two flavors, two x values, and two scales.
     *
     * @param flavor1 First parton flavor.
     * @param flavor2 Second parton flavor.
     * @param x1 First momentum fraction.
     * @param mu1_2 First factorization scale squared.
     * @param x2 Second momentum fraction.
     * @param mu2_2 Second factorization scale squared.
     *
     * @return DPD value.
     */
    double dpd(PartonFlavor flavor1, PartonFlavor flavor2, double x1, double mu1_2, double x2,
               double mu2_2) const
    {
        return dpdOperation_(pimpl_.get(), flavor1, flavor2, x1, mu1_2, x2, mu2_2);
    }

    ICDPD(const ICDPD &other)
        : pimpl_(other.clone_(other.pimpl_.get()), other.pimpl_.get_deleter()),
          clone_(other.clone_), dpdOperation_(other.dpdOperation_)
    {
    }

    ICDPD &operator=(const ICDPD &other)
    {
        if (this == &other)
            return *this;

        ICDPD copy(other);

        using std::swap;
        swap(pimpl_, copy.pimpl_);
        swap(clone_, copy.clone_);
        swap(dpdOperation_, copy.dpdOperation_);

        return *this;
    }

    ICDPD(ICDPD &&other) noexcept = default;
    ICDPD &operator=(ICDPD &&other) noexcept = default;

    ~ICDPD() = default;

  private:
    template <typename DPDApproachT> struct OwningModel
    {
        explicit OwningModel(DPDApproachT dpdApproach) : dpdApproach_(std::move(dpdApproach))
        {
        }

        double dpd(PartonFlavor flavor1, PartonFlavor flavor2, double x1, double mu1_2, double x2,
                   double mu2_2)
        {
            return dpdApproach_.dpd(flavor1, flavor2, x1, mu1_2, x2, mu2_2);
        }

        DPDApproachT dpdApproach_;
    };

    using DestroyOperation = void(void *);
    using CloneOperation = void *(void *);
    using DPDOperation = double(void *, PartonFlavor, PartonFlavor, double, double, double, double);

    std::unique_ptr<void, DestroyOperation *> pimpl_;
    CloneOperation *clone_{nullptr};
    DPDOperation *dpdOperation_{nullptr};
};

} // namespace PDFxTMD