module FactoryInterface_mod
    use iso_c_binding
    implicit none

    interface
        ! Coupling Factory interfaces
        function create_coupling_factory() bind(C, name="create_coupling_factory")
            use iso_c_binding
            type(c_ptr) :: create_coupling_factory
        end function create_coupling_factory

        subroutine delete_coupling_factory(factory) bind(C, name="delete_coupling_factory")
            use iso_c_binding
            type(c_ptr), value :: factory
        end subroutine delete_coupling_factory

        function mk_coupling(factory, pdfSetName) bind(C, name="mk_coupling")
            use iso_c_binding
            type(c_ptr) :: mk_coupling
            type(c_ptr), value :: factory
            character(kind=c_char), dimension(*) :: pdfSetName
        end function mk_coupling

        ! TMD Factory interfaces
        function create_tmd_factory() bind(C, name="create_tmd_factory")
            use iso_c_binding
            type(c_ptr) :: create_tmd_factory
        end function create_tmd_factory

        subroutine delete_tmd_factory(factory) bind(C, name="delete_tmd_factory")
            use iso_c_binding
            type(c_ptr), value :: factory
        end subroutine delete_tmd_factory

        function mk_tmd(factory, pdfSetName, setMember) bind(C, name="mk_tmd")
            use iso_c_binding
            type(c_ptr) :: mk_tmd
            type(c_ptr), value :: factory
            character(kind=c_char), dimension(*) :: pdfSetName
            integer(c_int), value :: setMember
        end function mk_tmd

        ! CPDF Factory interfaces
        function create_cpdf_factory() bind(C, name="create_cpdf_factory")
            use iso_c_binding
            type(c_ptr) :: create_cpdf_factory
        end function create_cpdf_factory

        subroutine delete_cpdf_factory(factory) bind(C, name="delete_cpdf_factory")
            use iso_c_binding
            type(c_ptr), value :: factory
        end subroutine delete_cpdf_factory

        function mk_cpdf(factory, pdfSetName, setMember) bind(C, name="mk_cpdf")
            use iso_c_binding
            type(c_ptr) :: mk_cpdf
            type(c_ptr), value :: factory
            character(kind=c_char), dimension(*) :: pdfSetName
            integer(c_int), value :: setMember
        end function mk_cpdf

        ! Add the interface for pdf function
        function cpdf_pdf(cpdf, flavor, x, mu2) bind(C, name="cpdf_pdf")
            use iso_c_binding
            real(c_double) :: cpdf_pdf
            type(c_ptr), value :: cpdf
            integer(c_int), value :: flavor
            real(c_double), value :: x
            real(c_double), value :: mu2
        end function cpdf_pdf
    end interface

end module FactoryInterface_mod 