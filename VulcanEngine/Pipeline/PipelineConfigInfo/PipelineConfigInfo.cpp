// // dix
// #include <Pipeline/EngineDevice/EngineDevice.hpp>
// #include <Pipeline/PipelineConfigInfo/PipelineConfigInfo.hpp>

// namespace dix {

// vk::PipelineVertexInputStateCreateInfo createVertexInputState(
//     const std::vector<vk::VertexInputBindingDescription>& bindingDescriptions,
//     const std::vector<vk::VertexInputAttributeDescription>&
//         attributeDescriptions) {
//     vk::PipelineVertexInputStateCreateInfo createInfo{};
//     createInfo.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
//     createInfo.vertexBindingDescriptionCount =
//         static_cast<uint32_t>(bindingDescriptions.size());
//     createInfo.pVertexBindingDescriptions = bindingDescriptions.data();
//     createInfo.vertexAttributeDescriptionCount =
//         static_cast<uint32_t>(attributeDescriptions.size());
//     createInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

//     return createInfo;
// }

// vk::PipelineInputAssemblyStateCreateInfo createInputAssemblyState(
//     vk::PrimitiveTopology topology) {
//     vk::PipelineInputAssemblyStateCreateInfo createInfo{};
//     createInfo.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
//     createInfo.topology = topology;

//     return createInfo;
// }

// std::pair<vk::Viewport, vk::Rect2D> createViewportAndScissor(int width,
//                                                              int height) {
//     vk::Viewport viewport{};
//     viewport.x = 0.0f;
//     viewport.y = 0.0f;
//     viewport.width = static_cast<float>(width);
//     viewport.height = static_cast<float>(height);
//     viewport.minDepth = 0.0f;
//     viewport.maxDepth = 1.0f;

//     vk::Rect2D scissor{};
//     scissor.offset = vk::Offset2D{0, 0};
//     scissor.extent = vk::Extent2D{static_cast<uint32_t>(width),
//                                   static_cast<uint32_t>(height)};

//     return std::make_pair(viewport, scissor);
// }

// vk::PipelineRasterizationStateCreateInfo createRasterizationState(
//     vk::PolygonMode polygonMode, bool cullModeBack) {
//     vk::PipelineRasterizationStateCreateInfo createInfo{};
//     createInfo.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
//     createInfo.depthClampEnable = vk::False;
//     createInfo.rasterizerDiscardEnable = vk::False;
//     createInfo.polygonMode = polygonMode;
//     createInfo.lineWidth = 1.0f;
//     createInfo.cullMode = cullModeBack ? vk::CullModeFlagBits::eBack
//                                        : vk::CullModeFlagBits::eNone;
//     createInfo.frontFace = vk::FrontFace::eClockwise;
//     createInfo.depthBiasEnable = vk::False;

//     return createInfo;
// }

// vk::PipelineMultisampleStateCreateInfo createMultisampleState() {
//     vk::PipelineMultisampleStateCreateInfo createInfo{};
//     createInfo.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
//     createInfo.sampleShadingEnable = vk::False;
//     createInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;

//     return createInfo;
// }

// vk::PipelineDepthStencilStateCreateInfo createDepthStencilState(
//     bool enableDepthTesting, bool enableDepthWriting, vk::CompareOp compareOp) {
//     vk::PipelineDepthStencilStateCreateInfo createInfo{};
//     createInfo.sType = vk::StructureType::ePipelineDepthStencilStateCreateInfo;
//     createInfo.depthTestEnable = enableDepthTesting ? vk::True : vk::False;
//     createInfo.depthWriteEnable = enableDepthWriting ? vk::True : vk::False;
//     createInfo.depthCompareOp = compareOp;
//     createInfo.minDepthBounds = 0.0f;  // Optional
//     createInfo.maxDepthBounds = 1.0f;  // Optional
//     createInfo.stencilTestEnable = vk::False;

//     return createInfo;
// }

// vk::PipelineColorBlendAttachmentState createColorBlendAttachmentState(
//     bool blendEnable) {
//     vk::PipelineColorBlendAttachmentState attachment{};
//     attachment.colorWriteMask =
//         vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
//         vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
//     attachment.blendEnable = blendEnable ? vk::True : vk::False;

//     if (blendEnable) {
//         attachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
//         attachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
//         attachment.colorBlendOp = vk::BlendOp::eAdd;
//         attachment.srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
//         attachment.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
//         attachment.alphaBlendOp = vk::BlendOp::eAdd;
//     } else {
//         attachment.srcColorBlendFactor = vk::BlendFactor::eOne;
//         attachment.dstColorBlendFactor = vk::BlendFactor::eZero;
//         attachment.colorBlendOp = vk::BlendOp::eAdd;
//         attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
//         attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
//         attachment.alphaBlendOp = vk::BlendOp::eAdd;
//     }

//     return attachment;
// }

// vk::PipelineColorBlendStateCreateInfo createColorBlendState(
//     const std::vector<vk::PipelineColorBlendAttachmentState>& attachments) {
//     vk::PipelineColorBlendStateCreateInfo createInfo{};
//     createInfo.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
//     createInfo.logicOpEnable = vk::False;
//     createInfo.logicOp = vk::LogicOp::eCopy;
//     createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//     createInfo.pAttachments = attachments.data();
//     createInfo.blendConstants[0] = 0.0f;
//     createInfo.blendConstants[1] = 0.0f;
//     createInfo.blendConstants[2] = 0.0f;
//     createInfo.blendConstants[3] = 0.0f;

//     return createInfo;
// }
// }  // namespace dix