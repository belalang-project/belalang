use proc_macro::TokenStream;
use quote::quote;
use syn::{
    Fields,
    ItemStruct,
    Token,
    parse::{
        Parse,
        ParseStream,
    },
    parse_macro_input,
};

// TODO: add documentation and clear namings

struct BelalangObjectArgs {
    name: syn::LitStr,
}

impl Parse for BelalangObjectArgs {
    fn parse(input: ParseStream) -> syn::Result<Self> {
        let mut name = None;

        while !input.is_empty() {
            let ident: syn::Ident = input.parse()?;
            input.parse::<Token![=]>()?;

            match ident.to_string().as_str() {
                "name" => {
                    if name.is_some() {
                        return Err(syn::Error::new(ident.span(), "duplicate `name` argument"));
                    }
                    name = Some(input.parse()?);
                },
                _ => return Err(syn::Error::new(ident.span(), "unknown argument")),
            }

            if !input.is_empty() {
                input.parse::<Token![,]>()?;
            }
        }

        Ok(BelalangObjectArgs {
            name: name.ok_or_else(|| syn::Error::new(input.span(), "missing `name` argument"))?,
        })
    }
}

#[proc_macro_attribute]
pub fn belalang_object(attr: TokenStream, item: TokenStream) -> TokenStream {
    let input = parse_macro_input!(item as ItemStruct);
    let args = parse_macro_input!(attr as BelalangObjectArgs);

    let Fields::Named(fields) = &input.fields else {
        return syn::Error::new_spanned(input, "belalang_type attribute only supports structs with named fields")
            .to_compile_error()
            .into();
    };

    let struct_name = &input.ident;
    let attrs = &input.attrs;
    let vis = &input.vis;
    let generics = &input.generics;
    let named_fields = &fields.named;
    let type_name = args.name;

    TokenStream::from(quote! {
        #(#attrs)*
        #[repr(C)]
        #[derive(Debug)]
        #vis struct #struct_name #generics {
            pub base: crate::BelalangBase,
            #named_fields
        }

        impl crate::BelalangObject for #struct_name {
            fn type_name() -> String {
                #type_name.into()
            }

            fn as_any(&self) -> &dyn std::any::Any {
                self
            }
        }
    })
}
