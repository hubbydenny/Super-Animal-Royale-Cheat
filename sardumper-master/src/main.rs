mod memory;
mod patterns;

use std::{
    fs::{self},
    path::Path,
};

use anyhow::{bail, Result};
use memory::*;
use object::read::coff::*;
use object::read::pe::*;
use object::{pe::*, LittleEndian};

fn dump(path: &Path) -> Result<()> {
    let data = fs::read(path)?;
    let data = data.as_slice();

    let dos_header = ImageDosHeader::parse(data)?;
    let mut offset = dos_header.nt_headers_offset().into();
    let (nt64_headers, _) = ImageNtHeaders64::parse(data, &mut offset)?;
    let header = nt64_headers.file_header();
    let il2cpp_section = match header
        .sections(data, offset)?
        .section_by_name(header.symbols(data)?.strings(), "il2cpp".as_bytes())
    {
        Some(v) => v.1,
        None => bail!("Il2cpp section not found"),
    };

    let va = il2cpp_section.virtual_address.get(LittleEndian) as usize;
    let code = il2cpp_section.pe_data(data)?;

    let scanner = memory::Scanner::new(code.as_ptr().cast(), code.len());

    unsafe {
        let try_scan = |name: &str, result: std::result::Result<usize, _>| -> String {
            match result {
                Ok(idx) => format!("{}:{}", name, va + idx),
                Err(_) => format!("{}:NOT_FOUND", name),
            }
        };

        let try_scan_rva = |name: &str, result: std::result::Result<usize, _>, ins_size: isize| -> String {
            match result {
                Ok(idx) => format!("{}:{}", name, get_virtual_absolute_address(va as _, code, idx as _, ins_size)),
                Err(_) => format!("{}:NOT_FOUND", name),
            }
        };

        println!("{}", try_scan("ReplaceSquare", scanner.find(patterns::REPLACE_SQUARE)));
        println!("{}", try_scan("PerfectHop", scanner.find(patterns::PERFECT_HOP)));
        println!("{}", try_scan_rva("AccelOffset", scanner.find(patterns::ACCEL_OFFSET), 4));
        println!("{}", try_scan("ReplaceZoomOffset", scanner.find(patterns::REPLACE_ZOOM_OFFSET)));
        println!("{}", try_scan("PlayerVision", scanner.find(patterns::PLAYER_VISION)));
        println!("{}", try_scan_rva("BananaStun", scanner.find(patterns::BANANA_STUN), 1));
        println!("{}", try_scan("RecoilPtr", scanner.find(patterns::RECOIL_PTR)));
        println!("{}", try_scan("ReplaceSpineOffset", scanner.find(patterns::REPLACE_SPINE_OFFSET)));
        println!("{}", try_scan_rva("SpeedOffset", scanner.find(patterns::SPEED_OFFSET), 4));
        println!("{}", try_scan_rva("ZoomSpeedOffset", scanner.find(patterns::ZOOM_SPEED_OFFSET), 4));
        println!("{}", try_scan("ItemVision", scanner.find(patterns::ITEM_VISION)));

        println!();
        println!("=== Hook targets ===");
        println!("{}", try_scan("NetworkPlayerDestroy", scanner.find(patterns::NETWORK_PLAYER_DESTROY)));
        println!("{}", try_scan("NetworkPlayerStart", scanner.find(patterns::NETWORK_PLAYER_START)));
        println!("{}", try_scan("WorldToScreenPoint", scanner.find(patterns::WORLD_TO_SCREEN_POINT)));
        println!("{}", try_scan("LocalPlayerStart", scanner.find(patterns::LOCAL_PLAYER_START)));
    }

    Ok(())
}

fn main() {
    let args = std::env::args().collect::<Vec<String>>();
    if args.len() != 2 {
        eprintln!(
            "Wrong args passed\nExample usage:\n\tsar-dumper.exe \"<path to Super Animal Royale GameAssembly.dll>\""
        );
        return;
    }

    let game_assembly_path = args[1].clone();
    if !game_assembly_path.ends_with("GameAssembly.dll") {
        eprintln!("Invalid path\nExample usage:\n\tsar-dumper.exe \"<path to Super Animal Royale GameAssembly.dll>\"");
        return;
    }

    match dump(Path::new(&game_assembly_path)) {
        Err(e) => eprintln!("Dump error:\n{}", e),
        _ => (),
    };
}
