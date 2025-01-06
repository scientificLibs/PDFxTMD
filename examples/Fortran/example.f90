program test_factory
    use FactoryInterface_mod
    use iso_c_binding
    implicit none

    type(c_ptr) :: cpdfFactory, cpdf
    character(len=100, kind=c_char) :: pdfSetName
    integer(c_int) :: setMember, flavor
    real(c_double) :: x, mu2, result

    pdfSetName = "CJ12min"//c_null_char
    setMember = 0

    ! Create factory and CPDF object
    cpdfFactory = create_cpdf_factory()
    cpdf = mk_cpdf(cpdfFactory, pdfSetName, setMember)

    ! Example parameters for pdf evaluation
    flavor = 2    ! For up quark
    x = 0.1d0     ! x value
    mu2 = 10.0d0  ! mu2 value

    ! Call the pdf function
    result = cpdf_pdf(cpdf, flavor, x, mu2)
    print *, "PDF value for flavor=", flavor, " at x=", x, " mu2=", mu2, " is:", result

    ! Clean up
    call delete_cpdf_factory(cpdfFactory)

end program test_factory