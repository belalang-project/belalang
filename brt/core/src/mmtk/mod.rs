use std::{
    ops::Range,
    sync::OnceLock,
};

use mmtk::{
    MMTK,
    MMTKBuilder,
    memory_manager,
    plan::AllocationSemantics,
    util::{
        Address,
        ObjectReference,
        copy::*,
        opaque_pointer::*,
    },
    vm::*,
};

static MMTK_INSTANCE: OnceLock<&'static MMTK<BrVM>> = OnceLock::new();

pub fn init() {
    // SAFETY: This is called once at startup before any other threads are spawned.
    // Setting the env var before mmtk_init ensures env_logger picks up Off level,
    // suppressing all MMTk log output without needing a custom logger.
    unsafe { std::env::set_var("RUST_LOG", "off") };

    let mut builder = MMTKBuilder::new();
    builder.options.plan.set(mmtk::util::options::PlanSelector::NoGC);
    let mmtk = memory_manager::mmtk_init::<BrVM>(&builder);
    MMTK_INSTANCE.set(Box::leak(mmtk)).ok();
}

pub fn alloc(size: usize) -> *mut u8 {
    let size = size.max(mmtk::util::constants::MIN_OBJECT_SIZE);

    let mmtk = MMTK_INSTANCE.get().expect("MMTk not initialized");
    let tls = VMMutatorThread(VMThread(OpaquePointer::UNINITIALIZED));
    let mut mutator = memory_manager::bind_mutator(mmtk, tls);
    let addr = memory_manager::alloc(&mut mutator, size, 8, 0, AllocationSemantics::Default);
    addr.to_mut_ptr::<u8>()
}

#[derive(Default)]
struct BrVM;

unsafe impl Sync for BrVM {}
unsafe impl Send for BrVM {}

impl VMBinding for BrVM {
    type VMSlot = Address;
    type VMMemorySlice = Range<Address>;

    type VMActivePlan = BrVM;
    type VMCollection = BrVM;
    type VMObjectModel = BrVM;
    type VMReferenceGlue = BrVM;
    type VMScanning = BrVM;

    const MAX_ALIGNMENT: usize = 1 << 6;
}

impl ActivePlan<BrVM> for BrVM {
    fn number_of_mutators() -> usize {
        1
    }

    fn is_mutator(_tls: VMThread) -> bool {
        true
    }

    fn mutator(_tls: VMMutatorThread) -> &'static mut mmtk::Mutator<BrVM> {
        unreachable!()
    }

    fn mutators<'a>() -> Box<dyn Iterator<Item = &'a mut mmtk::Mutator<BrVM>> + 'a> {
        unreachable!()
    }
}

impl Collection<BrVM> for BrVM {
    fn stop_all_mutators<F>(_tls: VMWorkerThread, _mutator_visitor: F)
    where
        F: FnMut(&'static mut mmtk::Mutator<BrVM>),
    {
    }

    fn resume_mutators(_tls: VMWorkerThread) {}

    fn block_for_gc(_tls: VMMutatorThread) {}

    fn spawn_gc_thread(_tls: VMThread, _ctx: GCThreadContext<BrVM>) {}
}

impl ObjectModel<BrVM> for BrVM {
    const GLOBAL_LOG_BIT_SPEC: VMGlobalLogBitSpec = VMGlobalLogBitSpec::in_header(0);
    const LOCAL_FORWARDING_POINTER_SPEC: VMLocalForwardingPointerSpec = VMLocalForwardingPointerSpec::in_header(0);
    const LOCAL_FORWARDING_BITS_SPEC: VMLocalForwardingBitsSpec = VMLocalForwardingBitsSpec::in_header(0);
    const LOCAL_MARK_BIT_SPEC: VMLocalMarkBitSpec = VMLocalMarkBitSpec::in_header(0);
    const LOCAL_LOS_MARK_NURSERY_SPEC: VMLocalLOSMarkNurserySpec = VMLocalLOSMarkNurserySpec::in_header(0);

    const OBJECT_REF_OFFSET_LOWER_BOUND: isize = 0;

    fn copy(
        _from: ObjectReference,
        _semantics: CopySemantics,
        _copy_context: &mut GCWorkerCopyContext<BrVM>,
    ) -> ObjectReference {
        unreachable!()
    }

    fn copy_to(_from: ObjectReference, _to: ObjectReference, _region: Address) -> Address {
        unreachable!()
    }

    fn get_current_size(_object: ObjectReference) -> usize {
        0
    }

    fn get_size_when_copied(_object: ObjectReference) -> usize {
        unreachable!()
    }

    fn get_align_when_copied(_object: ObjectReference) -> usize {
        unreachable!()
    }

    fn get_align_offset_when_copied(_object: ObjectReference) -> usize {
        unreachable!()
    }

    fn get_type_descriptor(_reference: ObjectReference) -> &'static [i8] {
        &[]
    }

    fn get_reference_when_copied_to(_from: ObjectReference, _to: Address) -> ObjectReference {
        unreachable!()
    }

    fn ref_to_object_start(object: ObjectReference) -> Address {
        object.to_raw_address()
    }

    fn ref_to_header(object: ObjectReference) -> Address {
        object.to_raw_address()
    }

    fn dump_object(_object: ObjectReference) {
        unreachable!()
    }
}

impl ReferenceGlue<BrVM> for BrVM {
    type FinalizableType = ObjectReference;

    fn clear_referent(_new_reference: ObjectReference) {}

    fn set_referent(_reference: ObjectReference, _referent: ObjectReference) {}

    fn get_referent(_object: ObjectReference) -> Option<ObjectReference> {
        None
    }

    fn enqueue_references(_references: &[ObjectReference], _tls: VMWorkerThread) {}
}

impl Scanning<BrVM> for BrVM {
    fn scan_object<SV: SlotVisitor<<BrVM as VMBinding>::VMSlot>>(
        _tls: VMWorkerThread,
        _object: ObjectReference,
        _slot_visitor: &mut SV,
    ) {
    }

    fn notify_initial_thread_scan_complete(_partial_scan: bool, _tls: VMWorkerThread) {}

    fn scan_roots_in_mutator_thread(
        _tls: VMWorkerThread,
        _mutator: &'static mut mmtk::Mutator<Self>,
        _factory: impl RootsWorkFactory<<BrVM as VMBinding>::VMSlot>,
    ) {
    }

    fn scan_vm_specific_roots(_tls: VMWorkerThread, _factory: impl RootsWorkFactory<<BrVM as VMBinding>::VMSlot>) {}

    fn supports_return_barrier() -> bool {
        false
    }

    fn prepare_for_roots_re_scanning() {}
}
